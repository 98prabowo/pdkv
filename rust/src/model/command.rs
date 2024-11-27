use super::error::{Error, Result};

pub enum Command {
    GET(String),
    SET{ key: String, value: String },
    DELETE(String),
}

impl Command {
    pub fn new(cmd: &str) -> Result<Self> {
        let cmd_lines: Vec<&str> = cmd.split_whitespace().collect();

        if cmd_lines.len() <= 1 && cmd_lines.len() > 3 {
            return Err(Error::UnknownCommand)
        }

        let cmd = cmd_lines.get(0).ok_or(Error::UnknownCommand)?;
        let key = cmd_lines.get(1);
        let value = cmd_lines.get(2);

        match cmd.to_lowercase().as_str() {
            "get" => Ok(Self::GET(key.ok_or(Error::KeyNotFound)?.to_string())),
            "set" => {
                if let Some(value) = value {
                    return Ok(Self::SET { 
                        key: key.ok_or(Error::KeyNotFound)?.to_string(), 
                        value: value.to_string() 
                    })
                } else {
                    return Err(Error::ValueNotFound) 
                }
            }
            "del" | "delete" => Ok(Self::DELETE(key.ok_or(Error::KeyNotFound)?.to_string())),
            _ => Err(Error::UnknownCommand),
        }
    }
}
