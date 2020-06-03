//! This module contains a virtual filesystem backed by a SLF file.
#![allow(dead_code)]

use std::convert::TryFrom;
use std::fmt;
use std::io;
use std::io::{Seek, SeekFrom};
use std::path::{Path, PathBuf};
use std::sync::{Arc, Mutex};

use crate::file_formats::slf::{SlfEntryState, SlfHeader};
use crate::fs::File;
use crate::math::checked_add_u64_i64;
use crate::unicode::Nfc;

/// A read-only case-insensitive virtual filesystem backed by a SLF file.
#[derive(Debug)]
pub struct SlfFs {
    /// Display info.
    pub slf_path: PathBuf,
    /// SLF archive open for reading.
    pub slf_file: Arc<Mutex<File>>,
    /// Case-insensitive base path.
    pub prefix: Nfc,
    /// List of entries
    pub entries: Vec<SlfFsEntry>,
}

/// A file entry.
#[derive(Debug)]
pub struct SlfFsEntry {
    /// Case-insensitive path from the base path.
    pub path: Nfc,
    /// Start of the data.
    pub offset: u32,
    /// Length of the data.
    pub length: u32,
}

/// A virtual file.
#[derive(Debug)]
pub struct SlfFsFile {
    /// Display info.
    pub file_path: Nfc,
    /// Display info.
    pub slf_path: PathBuf,
    /// SLF archive open for reading.
    pub slf_file: Arc<Mutex<File>>,
    /// Start of the data.
    pub offset: u32,
    /// Length of the data.
    pub length: u32,
    /// Current position.
    pub position: u64,
}

impl SlfFs {
    /// Creates a new virtual filesystem.
    pub fn new(path: &Path) -> io::Result<SlfFs> {
        let mut slf_file = File::open(&path)?;
        let header = SlfHeader::from_input(&mut slf_file)?;
        let entries: Vec<_> = header
            .entries_from_input(&mut slf_file)?
            .into_iter()
            .filter(|x| x.state == SlfEntryState::Ok)
            .map(|x| SlfFsEntry {
                path: Nfc::caseless_path(&x.file_path),
                offset: x.offset,
                length: x.length,
            })
            .collect();
        Ok(SlfFs {
            slf_path: path.to_owned(),
            slf_file: Arc::new(Mutex::new(slf_file)),
            prefix: Nfc::caseless_path(&header.library_path),
            entries,
        })
    }

    /// Opens a file in the filesystem.
    pub fn open(&self, file_path: &Nfc) -> io::Result<SlfFsFile> {
        if file_path.starts_with(self.prefix.as_str()) {
            let (_, want) = file_path.split_at(self.prefix.len());
            let entry_option = self
                .entries
                .iter()
                .filter(|x| x.path.as_str() == want)
                .nth(0);
            if let Some(entry) = entry_option {
                return Ok(SlfFsFile {
                    file_path: file_path.to_owned(),
                    slf_path: self.slf_path.to_owned(),
                    slf_file: self.slf_file.to_owned(),
                    offset: entry.offset,
                    length: entry.length,
                    position: 0,
                });
            }
        }
        Err(io::ErrorKind::NotFound.into())
    }
}

impl SlfFsFile {
    /// Gets the length of the file.
    pub fn len(&self) -> u64 {
        u64::from(self.length)
    }

    /// Returns true if the file is empty.
    pub fn is_empty(&self) -> bool {
        self.length == 0
    }
}

impl fmt::Display for SlfFs {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str("SlfFs { ")?;
        write!(f, "{:?}", self.slf_path)?;
        f.write_str(" }")
    }
}

impl fmt::Display for SlfFsFile {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str("SlfFsFile { ")?;
        write!(f, "{:?} in {:?}", self.file_path, self.slf_path)?;
        f.write_str(" }")
    }
}

impl io::Read for SlfFsFile {
    fn read(&mut self, mut buf: &mut [u8]) -> io::Result<usize> {
        let mut slf_file = self.slf_file.lock().expect("slf_file");
        let available = u64::from(self.length)
            .checked_sub(self.position)
            .unwrap_or(0);
        if let Ok(available) = usize::try_from(available) {
            if buf.len() > available {
                buf = &mut buf[..available];
            }
        }
        slf_file.seek(SeekFrom::Start(self.position + u64::from(self.offset)))?;
        let read_result = slf_file.read(buf);
        if let Ok(bytes) = read_result {
            self.position += u64::try_from(bytes).expect("u64");
        }
        read_result
    }
}

impl io::Seek for SlfFsFile {
    fn seek(&mut self, pos: SeekFrom) -> io::Result<u64> {
        let position_option = match pos {
            SeekFrom::Start(n) => Some(n),
            SeekFrom::Current(n) => checked_add_u64_i64(self.position, n),
            SeekFrom::End(n) => checked_add_u64_i64(u64::from(self.length), n),
        };
        if let Some(position) = position_option {
            if position.checked_add(u64::from(self.offset)).is_some() {
                self.position = position;
                return Ok(position);
            }
        }
        // underflow or overflow
        Err(io::Error::from(io::ErrorKind::InvalidInput))
    }
}

impl io::Write for SlfFsFile {
    fn write(&mut self, _buf: &[u8]) -> io::Result<usize> {
        Err(io::Error::new(
            io::ErrorKind::PermissionDenied,
            "stracciatella::vfs::slf::SlfFsFile is read-only",
        ))
    }
    fn flush(&mut self) -> io::Result<()> {
        Err(io::Error::new(
            io::ErrorKind::PermissionDenied,
            "stracciatella::vfs::slf::SlfFsFile is read-only",
        ))
    }
}
