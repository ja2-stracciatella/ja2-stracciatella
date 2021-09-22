use std::{
    fs::File,
    io::{BufReader, Read, Seek, Write},
};

use crate::vfs::VfsFile;
trait SeekRead: Seek + Read {}
impl<T: Seek + Read> SeekRead for T {}

pub enum VFile {
    VfsFile(BufReader<Box<dyn VfsFile>>),
    File(File),
    BufFile(BufReader<File>),
}

impl From<File> for VFile {
    fn from(f: File) -> Self {
        VFile::File(f)
    }
}

impl From<Box<dyn VfsFile>> for VFile {
    fn from(f: Box<dyn VfsFile>) -> Self {
        VFile::VfsFile(BufReader::new(f))
    }
}

impl VFile {
    pub fn buf_file(f: File) -> Self {
        VFile::BufFile(BufReader::new(f))
    }

    pub fn len(&self) -> std::io::Result<u64> {
        match self {
            VFile::VfsFile(file) => file.get_ref().len(),
            VFile::File(file) => file.metadata().map(|m| m.len()),
            VFile::BufFile(file) => file.get_ref().metadata().map(|m| m.len()),
        }
    }

    pub fn is_empty(&self) -> std::io::Result<bool> {
        self.len().map(|l| l == 0)
    }
}

impl Read for VFile {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        match self {
            VFile::VfsFile(file) => file.read(buf),
            VFile::File(file) => file.read(buf),
            VFile::BufFile(read) => read.read(buf),
        }
    }
}

impl Write for VFile {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        match self {
            VFile::File(file) => file.write(buf),
            VFile::BufFile(_) | VFile::VfsFile(_) => Err(std::io::Error::new(
                std::io::ErrorKind::PermissionDenied,
                "Attempted to write to a file opened with read permissions",
            )),
        }
    }

    fn flush(&mut self) -> std::io::Result<()> {
        match self {
            VFile::File(file) => file.flush(),
            VFile::BufFile(_) | VFile::VfsFile(_) => Err(std::io::Error::new(
                std::io::ErrorKind::PermissionDenied,
                "Attempted to flush a file opened with read permissions",
            )),
        }
    }
}

impl Seek for VFile {
    fn seek(&mut self, pos: std::io::SeekFrom) -> std::io::Result<u64> {
        match self {
            VFile::VfsFile(file) => file.seek(pos),
            VFile::File(file) => file.seek(pos),
            VFile::BufFile(read) => read.seek(pos),
        }
    }
}
