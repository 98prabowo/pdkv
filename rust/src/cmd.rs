pub enum Command {
    GET(String),
    SET{ key: String, value: String },
    DELETE(String),
}

impl Command {
    pub fn new(cmd: &str) -> Option<Self> {
        let cmd_line: Vec<&str> = cmd.split_whitespace().collect();
        let cmd = cmd_line.get(0)?;
        let key = cmd_line.get(1)?;
        let value = cmd_line.get(2);

        match cmd.to_lowercase().as_str() {
            "get" => Some(Self::GET(key.to_string())),
            "set" => {
                if let Some(value) = value {
                    return Some(Self::SET { key: key.to_string(), value: value.to_string() })
                } else {
                    return None
                }
            }
            "del" | "delete" => Some(Self::DELETE(key.to_string())),
            _ => None,
        }
    }
}
