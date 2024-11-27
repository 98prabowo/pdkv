pub type Result<T> = std::result::Result<T, Error>;

#[derive(Debug)]
pub enum Error {
    KeyNotFound,
    ValueNotFound,
    UnknownCommand,
}

impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::KeyNotFound       => write!(f, "Key not found"),
            Self::ValueNotFound     => write!(f, "Value not found"),
            Self::UnknownCommand    => write!(f, "Unknown command"),
        }
    }
}

impl std::error::Error for Error {}
