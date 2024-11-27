use std::{
    io::{BufRead, BufReader, Write}, 
    net::TcpStream, 
};

use crate::model::{AtomicDB, Command};

pub struct Controller;

impl Controller {
    pub fn handle_input(
        stream: &mut TcpStream, 
        db: AtomicDB
    ) {
        let buf_reader = BufReader::new(&mut *stream);

        if let Some(Ok(request_line)) = buf_reader.lines().next() {
            let respond: String = Self::handle_cmd(db, request_line);
            stream.write_all(respond.as_bytes()).unwrap();
        }
    }

    fn handle_cmd(
        db: AtomicDB,
        request_line: String
    ) -> String {
        match Command::new(&request_line) {
            Ok(Command::GET(key))           => Controller::handle_get(db, key),
            Ok(Command::SET{ key, value })  => Controller::handle_set(db, key, value),
            Ok(Command::DELETE(key))        => Controller::handle_del(db, key),
            Err(error)                      => format!("{}\n", error),
        }
    }

    fn handle_get(
        db: AtomicDB,
        key: String
    ) -> String {
        match db.lock() {
            Ok(db) => {
                if let Some(value) = db.get(&key) {
                    format!("{}\n", value)
                } else {
                    format!("No value for key: {}\n", key)
                }
            }
            Err(_) => format!("Failed to get data\n"),
        }
    }

    fn handle_set(
        db: AtomicDB,
        key: String,
        value: String
    ) -> String {
        match db.lock() {
            Ok(mut db) => {
                if let Some(current_v) = db.get_mut(&key) {
                    *current_v = value
                } else { 
                    db.entry(key).or_insert(value);
                }
                "Success entry data\n".into()
            }
            Err(_) => "Failed to entry data\n".into(),
        }
    }

    fn handle_del(
        db: AtomicDB,
        key: String
    ) -> String {
        match db.lock() {
            Ok(mut db) => {
                if let Some(_) = db.remove(&key) {
                    format!("Success remove {}\n", key)
                } else {
                    format!("No value for key: {}\n", key)
                }
            }
            Err(_) => "Failed to delete data\n".into(),
        }
    }
}
