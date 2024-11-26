mod cmd;
mod controller;
mod error;
mod pool;
mod server;
mod worker;

use server::Server;

fn main() {
    let server = Server::init();
    server.run();
}
