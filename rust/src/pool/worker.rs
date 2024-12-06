use std::{
    sync::{mpsc::Receiver, Arc, Mutex},
    thread::{self, JoinHandle},
};

pub type Job = Box<dyn FnOnce() + Send + 'static>;

pub struct Worker {
    pub id: usize,
    pub thread: Option<JoinHandle<()>>,
}

impl Worker {
    pub fn new(id: usize, receiver: Arc<Mutex<Receiver<Job>>>) -> Self {
        let thread = thread::spawn(move || loop {
            let message = receiver
                .lock()
                .expect("Mutex got poisoned state")
                .recv();

            match message {
                Ok(job) => {
                    println!("Worker {id} got a job; executing.");
                    job();
                }
                Err(_) => {
                    println!("Worker {id} disconnected; shutting down.");
                    break;
                }
            }
        });

        let thread = Some(thread);

        Worker { id, thread }
    }
}

#[cfg(test)]
mod worker_tests {
    use std::{sync::mpsc::channel, time::Duration};

    use super::*;

    #[test]
    fn worker_process_job_test() {
        let (sender, receiver) = channel();
        let receiver = Arc::new(Mutex::new(receiver));
        let _sut = Worker::new(0, receiver);

        let (rsend, rrecv) = channel();
        let job = move || rsend.send(()).unwrap();
        let job = Box::new(job);

        sender.send(job).unwrap();

        assert_eq!(rrecv.recv_timeout(Duration::from_secs(1)), Ok(()));
    }
}
