use std::{error::Error, fmt::Display};

#[derive(Debug)]
pub enum KVError {
    InvalidSize
}

impl Display for KVError {
    fn fmt(&self, fmt: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::InvalidSize => write!(fmt, "Invalid Pool Size"),
        }
    }
}

impl Error for KVError {}
