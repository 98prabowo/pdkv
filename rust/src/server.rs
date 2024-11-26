use std::{
    collections::HashMap,
    net::TcpListener,
    sync::{Arc, Mutex},
};

use crate::{
    controller::Controller,
    pool::ThreadPool
};

pub struct Server {
    db: Arc<Mutex<HashMap<String, String>>>,
}

impl Server {
    pub fn init() -> Self {
        Self {
            db: Arc::new(Mutex::new(HashMap::new())),
        }
    }

    pub fn run(&self) {
        let listener = TcpListener::bind("127.0.0.1:31337").unwrap();
        let pool = ThreadPool::build(10).expect("Invalid thread pool size. Size must be more than zero");

        for stream in listener.incoming() {
            let mut stream = stream.unwrap();
            let db = Arc::clone(&self.db);

            pool.execute(move || {
                Controller::handle_cmd(&mut stream, db);
            });
        }
    }
}
