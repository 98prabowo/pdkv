use std::sync::{
    mpsc::{channel, Sender}, 
    Arc, 
    Mutex
};

use crate::{
    error::KVError, 
    worker::{Job, Worker}
};

pub struct ThreadPool {
    workers: Vec<Worker>,
    sender: Option<Sender<Job>>,
}

impl ThreadPool {
    pub fn build(size: usize) -> Result<Self, KVError> {
        if size <= 0 {
            return Err(KVError::InvalidSize)
        }

        let (sender, receiver) = channel();
        let sender = Some(sender);
        let receiver = Arc::new(Mutex::new(receiver));

        let mut workers = Vec::with_capacity(size);

        for id in 0..size {
            let worker = Worker::new(id, Arc::clone(&receiver));
            workers.push(worker);
        }

        Ok(Self { workers, sender })
    }

    pub fn execute<F>(&self, task: F)
    where 
        F: FnOnce() + Send + 'static
    {
        let job = Box::new(task);

        self.sender
            .as_ref()
            .expect("Sender is null")
            .send(job)
            .expect("Receiver has dropped");
    }
}

impl Drop for ThreadPool {
    fn drop(&mut self) {
        drop(self.sender.take());

        for worker in &mut self.workers {
            println!("Shutting down worker {}", worker.id);

            if let Some(thread) = worker.thread.take() {
                thread.join().unwrap();
            }
        }
    }
}
