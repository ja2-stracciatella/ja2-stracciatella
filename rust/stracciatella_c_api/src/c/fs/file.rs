//! This module contains the C interface for [`stracciatella::fs::File`].

use std::io;
use std::io::{Read, Seek, Write};
use std::ptr;
use std::u64;
use std::usize;

use stracciatella::fs;

use crate::c::common::*;

/// Opens the file for reading.
/// @see https://doc.rust-lang.org/std/fs/struct.OpenOptions.html#method.read
pub const FILE_OPEN_READ: u8 = 0x01;

/// Opens the file for writing.
/// @see https://doc.rust-lang.org/std/fs/struct.OpenOptions.html#method.write
pub const FILE_OPEN_WRITE: u8 = 0x02;

/// Opens the file for appending.
/// Setting WRITE and APPEND has the same effect as setting only APPEND.
/// @see https://doc.rust-lang.org/std/fs/struct.OpenOptions.html#method.append
pub const FILE_OPEN_APPEND: u8 = 0x04;

/// Truncates the file to 0 length.
/// @see https://doc.rust-lang.org/std/fs/struct.OpenOptions.html#method.truncate
pub const FILE_OPEN_TRUNCATE: u8 = 0x08;

/// Creates the file if it does not exist.
/// Needs WRITE or APPEND.
/// @see https://doc.rust-lang.org/std/fs/struct.OpenOptions.html#method.create
pub const FILE_OPEN_CREATE: u8 = 0x10;

/// Creates a new file or fails if it already exists.
/// CREATE and TRUNCATE will be ignored.
/// Needs WRITE or APPEND.
/// @see https://doc.rust-lang.org/std/fs/struct.OpenOptions.html#method.create_new
pub const FILE_OPEN_CREATE_NEW: u8 = 0x20;

/// A wrapper around [`File`] for C.
/// @see https://doc.rust-lang.org/std/fs/struct.File.html
pub struct File {
    inner: fs::File,
}

/// Opens a file according to the options.
/// Sets the rust error.
/// @see FILE_OPEN_*
#[no_mangle]
pub extern "C" fn File_open(path: *const c_char, options: u8) -> *mut File {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let file_result = fs::OpenOptions::new()
        .read((options & FILE_OPEN_READ) != 0)
        .write((options & FILE_OPEN_WRITE) != 0)
        .append((options & FILE_OPEN_APPEND) != 0)
        .truncate((options & FILE_OPEN_TRUNCATE) != 0)
        .create((options & FILE_OPEN_CREATE) != 0)
        .create_new((options & FILE_OPEN_CREATE_NEW) != 0)
        .open(&path);
    match file_result {
        Err(err) => {
            remember_rust_error(format!("File_open {:?} {:#02x}: {}", path, options, err));
            ptr::null_mut()
        }
        Ok(file) => into_ptr(File { inner: file }),
    }
}

/// Closes the file.
#[no_mangle]
pub extern "C" fn File_close(file: *mut File) {
    let _drop_me = from_ptr(file);
}

/// Returns the size of the file in bytes, or ´u64::MAX´ if there is an error.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/fs/struct.Metadata.html#method.len
#[no_mangle]
pub extern "C" fn File_len(file: *mut File) -> u64 {
    forget_rust_error();
    let file = unsafe_ref(file);
    match file.inner.metadata() {
        Err(err) => {
            remember_rust_error(format!("File_len: {}", err));
            u64::MAX
        }
        Ok(metadata) => metadata.len(),
    }
}

/// Reads data from the file to the buffer.
/// Returns the number of bytes read or ´usize::MAX´ if there is an error.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/io/trait.Read.html#tymethod.read
#[no_mangle]
pub extern "C" fn File_read(file: *mut File, buf: *mut u8, buf_len: usize) -> usize {
    forget_rust_error();
    let file = unsafe_mut(file);
    let buf = unsafe_slice_mut(buf, buf_len);
    loop {
        match file.inner.read(buf) {
            Err(err) => {
                if err.kind() != io::ErrorKind::Interrupted {
                    remember_rust_error(format!("File_read {}: {}", buf_len, err));
                    return usize::MAX;
                }
            }
            Ok(n) => return n,
        }
    }
}

/// Reads data from the file to the buffer until it is full.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/io/trait.Read.html#method.read_exact
#[no_mangle]
pub extern "C" fn File_readExact(file: *mut File, buf: *mut u8, buf_len: usize) -> bool {
    forget_rust_error();
    let file = unsafe_mut(file);
    let buf = unsafe_slice_mut(buf, buf_len);
    while let Err(err) = file.inner.read_exact(buf) {
        if err.kind() != io::ErrorKind::Interrupted {
            remember_rust_error(format!("File_readExact {}: {}", buf_len, err));
            break;
        }
    }
    no_rust_error()
}

/// Writes data from the buffer to the file.
/// Returns the number of bytes written or ´usize::MAX´ if there is an error.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/io/trait.Write.html#tymethod.write
#[no_mangle]
pub extern "C" fn File_write(file: *mut File, buf: *const u8, buf_len: usize) -> usize {
    forget_rust_error();
    let file = unsafe_mut(file);
    let buf = unsafe_slice(buf, buf_len);
    loop {
        match file.inner.write(buf) {
            Err(err) => {
                if err.kind() != io::ErrorKind::Interrupted {
                    remember_rust_error(format!("File_write {}: {}", buf_len, err));
                    return usize::MAX;
                }
            }
            Ok(n) => return n,
        }
    }
}

/// Writes all the data from the buffer to the file.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/io/trait.Write.html#method.write_all
#[no_mangle]
pub extern "C" fn File_writeAll(file: *mut File, buf: *const u8, buf_len: usize) -> bool {
    forget_rust_error();
    let file = unsafe_mut(file);
    let buf = unsafe_slice(buf, buf_len);
    while let Err(err) = file.inner.write_all(buf) {
        if err.kind() == io::ErrorKind::Interrupted {
            continue;
        }
        remember_rust_error(format!("File_writeAll {}: {}", buf_len, err));
    }
    no_rust_error()
}

/// Seeks to an offset relative to the start of the file.
/// Returns the resulting position or u64::MAX if there is an error.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/io/trait.Seek.html#tymethod.seek
/// @see https://doc.rust-lang.org/std/io/enum.SeekFrom.html#variant.Start
#[no_mangle]
pub extern "C" fn File_seekFromStart(file: *mut File, offset: u64) -> u64 {
    forget_rust_error();
    let file = unsafe_mut(file);
    match file.inner.seek(io::SeekFrom::Start(offset)) {
        Err(err) => {
            remember_rust_error(format!("File_seekFromStart {}: {}", offset, err));
            u64::MAX
        }
        Ok(position) => position,
    }
}

/// Seeks to an offset relative to the end of the file.
/// Returns the resulting position or u64::MAX if there is an error.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/io/trait.Seek.html#tymethod.seek
/// @see https://doc.rust-lang.org/std/io/enum.SeekFrom.html#variant.End
#[no_mangle]
pub extern "C" fn File_seekFromEnd(file: *mut File, offset: i64) -> u64 {
    forget_rust_error();
    let file = unsafe_mut(file);
    match file.inner.seek(io::SeekFrom::End(offset)) {
        Err(err) => {
            remember_rust_error(format!("File_seekFromEnd {}: {}", offset, err));
            u64::MAX
        }
        Ok(position) => position,
    }
}

/// Seeks to an offset relative to the current position in the file.
/// Returns the resulting position or u64::MAX if there is an error.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/io/trait.Seek.html#tymethod.seek
/// @see https://doc.rust-lang.org/std/io/enum.SeekFrom.html#variant.Current
#[no_mangle]
pub extern "C" fn File_seekFromCurrent(file: *mut File, offset: i64) -> u64 {
    forget_rust_error();
    let file = unsafe_mut(file);
    match file.inner.seek(io::SeekFrom::Current(offset)) {
        Err(err) => {
            remember_rust_error(format!("File_seekFromCurrent {}: {}", offset, err));
            u64::MAX
        }
        Ok(position) => position,
    }
}
