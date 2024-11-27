mod error;
mod worker;

use std::sync::{
    mpsc::{channel, Sender}, 
    Arc, 
    Mutex
};

use worker::{Job, Worker};

pub use error::{Error, Result};

pub struct ThreadPool {
    workers: Vec<Worker>,
    sender: Option<Sender<Job>>,
}

impl ThreadPool {
    pub fn build(size: usize) -> Result<Self> {
        if size <= 0 {
            return Err(Error::InvalidPoolSize)
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

    pub fn execute<F>(&self, task: F) -> Result<()>
    where 
        F: FnOnce() + Send + 'static
    {
        let job = Box::new(task);

        self.sender
            .as_ref()
            .ok_or(Error::SenderIsNil)?
            .send(job)
            .map_err(|_| Error::ReceiverIsDropped)?;

        Ok(())
    }
}

impl Drop for ThreadPool {
    fn drop(&mut self) {
        drop(self.sender.take());

        for worker in &mut self.workers {
            println!("Shutting down worker {}", worker.id);

            if let Some(thread) = worker.thread.take() {
                thread
                    .join()
                    .expect(format!("Couldn't join thread on worker {}", worker.id).as_str());
            }
        }
    }
}
