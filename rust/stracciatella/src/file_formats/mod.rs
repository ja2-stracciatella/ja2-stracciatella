//! This module contains code to read and write fileformats that are used by Jagged Alliance 2

use std::io::ErrorKind::{InvalidData, InvalidInput};
use std::io::{Error, Read, Result, Write};

pub mod slf;
pub mod stci;

/// Trait that adds extra functions to Read.
pub trait StracciatellaReadExt: Read {
    /// Reads and discards unused bytes.
    fn read_unused(&mut self, num_bytes: usize) -> Result<()> {
        let mut buffer = vec![0u8; num_bytes];
        self.read_exact(&mut buffer)?;
        Ok(())
    }

    /// Reads a nul terminated fixed size string.
    fn read_fixed_string(&mut self, num_bytes: usize) -> Result<String> {
        let mut buffer = vec![0u8; num_bytes];
        self.read_exact(&mut buffer)?;
        // must be nul terminated and valid utf8
        match buffer.iter().position(|&byte| byte == 0) {
            Some(position) => match ::std::str::from_utf8(&buffer[..position]) {
                Ok(s) => Ok(s.to_string()),
                Err(e) => Err(Error::new(InvalidData, e)),
            },
            None => Err(Error::new(InvalidData, "string is not nul terminated")),
        }
    }
}

/// Trait that adds extra functions to Write.
pub trait StracciatellaWriteExt: Write {
    /// Writes zeroed unused bytes.
    fn write_unused(&mut self, num_bytes: usize) -> Result<()> {
        let buffer = vec![0u8; num_bytes];
        self.write_all(&buffer)
    }

    /// Write a nul terminated fixed size string, unused space is zeroed.
    fn write_fixed_string(&mut self, num_bytes: usize, string: &str) -> Result<()> {
        let mut buffer = vec![0u8; num_bytes];
        let string_bytes = string.as_bytes();
        if string_bytes.len() >= buffer.len() {
            return Err(Error::new(InvalidInput, "string is too long"));
        }
        buffer[..string_bytes.len()].copy_from_slice(&string_bytes);
        self.write_all(&buffer)?;
        Ok(())
    }
}

/// Everything that implements Read gets Ja2WriteExt for free.
impl<T: Read + ?Sized> StracciatellaReadExt for T {}

/// Everything that implements Write gets Ja2WriteExt for free.
impl<T: Write + ?Sized> StracciatellaWriteExt for T {}
