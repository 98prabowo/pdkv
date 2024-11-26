use std::{
    collections::HashMap, 
    io::{BufRead, BufReader, Write}, 
    net::TcpStream, 
    sync::{Arc, Mutex}
};

use crate::cmd::Command;

pub struct Controller;

impl Controller {
    pub fn handle_cmd(stream: &mut TcpStream, db: Arc<Mutex<HashMap<String, String>>>) {
        let buf_reader = BufReader::new(&mut *stream);
        let request_line = buf_reader.lines().next().unwrap().unwrap();

        match Command::new(&request_line) {
            Some(Command::GET(key)) => {
                if let Some(value) = db.lock().unwrap().get(&key) {
                    stream.write_all(value.as_bytes()).unwrap();
                    stream.write_all("\n".as_bytes()).unwrap();
                } else {
                    let response = format!("No value for key: {}\n", key);
                    stream.write_all(response.as_bytes()).unwrap();
                }
            }
            Some(Command::SET{ key, value }) => {
                let mut db = db.lock().unwrap();

                if let Some(current_v) = db.get_mut(&key) {
                    *current_v = value
                } else { 
                    db.entry(key).or_insert(value);
                }

                stream.write_all("Success entry data\n".as_bytes()).unwrap();
            }
            Some(Command::DELETE(key)) => {
                db.lock().unwrap().remove(&key);
                let response = format!("Success remove {}\n", key);
                stream.write_all(response.as_bytes()).unwrap();
            }
            None => {
                stream.write_all("Command unknown\n".as_bytes()).unwrap();
            }
        }
    }
}
