//! This module contains the C interface for [`stracciatella::fs`].
//!
//! [`stracciatella::fs`]: ../../stracciatella/fs/index.html

use std::f64;
use std::io;
use std::io::{Read, Seek, Write};
use std::ptr;
use std::time;
use std::u64;
use std::usize;

use stracciatella::fs;

use crate::c::common::*;
use crate::c::vec::{VecCString, VecU8};

pub mod tempdir;

/// Creates a directory.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Fs_createDir(path: *const c_char) -> bool {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    if let Err(err) = fs::create_dir(&path) {
        remember_rust_error(format!("Fs_createDir {:?}: {}", path, err));
    }
    no_rust_error()
}

/// Checks if the path exists.
#[no_mangle]
pub extern "C" fn Fs_exists(path: *const c_char) -> bool {
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    fs::metadata(&path).is_ok()
}

/// Gets the free space in the target path.
/// On error the free space will be 0.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Fs_freeSpace(path: *const c_char, bytes: *mut u64) -> bool {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let bytes = unsafe_mut(bytes);
    let free_space = match fs::free_space(&path) {
        Err(err) => {
            remember_rust_error(format!("Fs_freeSpace {:?}: {}", path, err));
            0
        }
        Ok(n) => n,
    };
    *bytes = free_space;
    no_rust_error()
}

/// Checks if the path points to a directory.
#[no_mangle]
pub extern "C" fn Fs_isDir(path: *const c_char) -> bool {
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    fs::metadata(&path).map(|x| x.is_dir()).unwrap_or(false)
}

/// Checks if the path points to a file.
#[no_mangle]
pub extern "C" fn Fs_isFile(path: *const c_char) -> bool {
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    fs::metadata(&path).map(|x| x.is_file()).unwrap_or(false)
}

/// Gets the paths of the directory entries.
/// Returns null on error.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Fs_readDirPaths(
    dir: *const c_char,
    ignore_entry_errors: bool,
) -> *mut VecCString {
    forget_rust_error();
    let dir = path_buf_from_c_str_or_panic(unsafe_c_str(dir));
    match fs::read_dir_paths(&dir, ignore_entry_errors) {
        Err(err) => {
            remember_rust_error(format!("Fs_readDirPaths {:?}: {}", dir, err));
            ptr::null_mut()
        }
        Ok(vec) => {
            let vec: Vec<_> = vec
                .iter()
                .map(|x| c_string_from_path_or_panic(&x))
                .collect();
            let c_vec = VecCString::from(vec);
            into_ptr(c_vec)
        }
    }
}

/// Gets the modified time in seconds since the unix epoch.
/// On error the modified time will be the minimum value.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Fs_modifiedSecs(path: *const c_char, modified_secs: *mut f64) -> bool {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let modified_secs = unsafe_mut(modified_secs);
    // FIXME use Duration.as_secs_f64 with rust 1.38.0+
    let as_secs_f64 = |duration: time::Duration| {
        const NANOSECOND: f64 = 0.000_000_001;
        duration.as_secs() as f64 + f64::from(duration.subsec_nanos()) * NANOSECOND
    };
    let secs_result = fs::metadata(&path).and_then(|x| x.modified()).map(|x| {
        match x.duration_since(time::UNIX_EPOCH) {
            Ok(duration) => as_secs_f64(duration),
            Err(err) => -as_secs_f64(err.duration()),
        }
    });
    match secs_result {
        Err(err) => {
            remember_rust_error(format!("Fs_modifiedSecs {:?}: {}", path, err));
            *modified_secs = f64::MIN;
        }
        Ok(secs) => {
            *modified_secs = secs;
        }
    }
    no_rust_error()
}

/// Removes a directory and all it's contents.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/fs/fn.remove_dir_all.html
#[no_mangle]
pub extern "C" fn Fs_removeDirAll(dir: *const c_char) -> bool {
    forget_rust_error();
    let dir = path_buf_from_c_str_or_panic(unsafe_c_str(dir));
    if let Err(err) = fs::remove_dir_all(&dir) {
        remember_rust_error(format!("Fs_removeDirAll {:?}: {}", dir, err));
    }
    no_rust_error()
}

/// Renames a file or directory.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/fs/fn.rename.html
#[no_mangle]
pub extern "C" fn Fs_rename(from: *const c_char, to: *const c_char) -> bool {
    forget_rust_error();
    let from = path_buf_from_c_str_or_panic(unsafe_c_str(from));
    let to = path_buf_from_c_str_or_panic(unsafe_c_str(to));
    if let Err(err) = fs::rename(&from, &to) {
        remember_rust_error(format!("Fs_rename {:?} {:?}: {}", from, to, err));
    }
    no_rust_error()
}

/// Sets the readonly permissions of a file or directory.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Fs_setReadOnly(path: *const c_char, readonly: bool) -> bool {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let result = fs::metadata(&path).and_then(|x| {
        let mut permissions = x.permissions();
        permissions.set_readonly(readonly);
        fs::set_permissions(&path, permissions)
    });
    if let Err(err) = result {
        remember_rust_error(format!("Fs_setReadOnly {:?} {}: {}", path, readonly, err));
    }
    no_rust_error()
}

/// Reads all the bytes from a file.
/// Returns null if there is an error.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/fs/fn.read.html
#[no_mangle]
pub extern "C" fn Fs_read(path: *const c_char) -> *mut VecU8 {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    match fs::read(&path) {
        Err(err) => {
            remember_rust_error(format!("Fs_read {:?}: {}", path, err));
            ptr::null_mut()
        }
        Ok(vec) => into_ptr(VecU8::from(vec)),
    }
}

/// Writes the bytes to a file.
/// The file will be created if it does not exist.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/fs/fn.write.html
#[no_mangle]
pub extern "C" fn Fs_write(path: *const c_char, buf: *const u8, buf_len: usize) -> bool {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let buf = unsafe_slice(buf, buf_len);
    if let Err(err) = fs::write(&path, buf) {
        remember_rust_error(format!("Fs_write {:?} {}: {}", path, buf_len, err));
    }
    no_rust_error()
}

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
