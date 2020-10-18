//! This module contains the C interface for [`stracciatella::vfs`].
//!
//! [`stracciatella::vfs`]: ../../../stracciatella/vfs/index.html

use std::io::{Read, Seek, SeekFrom, Write};
use std::usize;

use stracciatella::config::EngineOptions;
use stracciatella::unicode::Nfc;
use stracciatella::vfs::{Vfs, VfsFile};

use crate::c::common::*;

/// Creates a virtual filesystem.
/// coverity[+alloc]
#[no_mangle]
pub extern "C" fn Vfs_create() -> *mut Vfs {
    into_ptr(Vfs::new())
}

/// Destroys the virtual filesystem.
/// coverity[+free : arg-0]
#[no_mangle]
pub extern "C" fn Vfs_destroy(vfs: *mut Vfs) {
    let _drop_me = from_ptr(vfs);
}

/// Initializes the VFS based on the information in engine_options
/// Returns true if successful, false otherwise.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Vfs_init_from_engine_options(
    vfs: *mut Vfs,
    engine_options: *const EngineOptions,
) -> bool {
    forget_rust_error();
    let vfs = unsafe_mut(vfs);
    let engine_options = unsafe_ref(engine_options);
    if let Err(err) = vfs.init_from_engine_options(engine_options) {
        remember_rust_error(format!(
            "Vfs_init_from_engine_options {:?}: {}",
            engine_options, err
        ));
    }
    no_rust_error()
}

/// Adds an overlay filesystem backed by a filesystem directory.
/// Returns true if successful, false otherwise.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Vfs_addDir(vfs: *mut Vfs, path: *const c_char) -> bool {
    forget_rust_error();
    let vfs = unsafe_mut(vfs);
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    if let Err(err) = vfs.add_dir(&path) {
        remember_rust_error(format!("Vfs_addDir {:?}: {}", path, err));
    }
    no_rust_error()
}

/// Opens a virtual file for reading.
/// Returns the file on success, null otherwise.
/// Sets the rust error.
/// coverity[+alloc]
#[no_mangle]
pub extern "C" fn VfsFile_open(vfs: *mut Vfs, path: *const c_char) -> *mut VfsFile {
    forget_rust_error();
    let vfs = unsafe_mut(vfs);
    let path = str_from_c_str_or_panic(unsafe_c_str(path));
    match vfs.open(&Nfc::caseless_path(&path)) {
        Err(err) => {
            remember_rust_error(format!("VfsFile_open {:?}: {}", path, err));
            std::ptr::null_mut()
        }
        Ok(file) => into_ptr(file),
    }
}

/// Closes the virtual file.
/// coverity[+free : arg-0]
#[no_mangle]
pub extern "C" fn VfsFile_close(file: *mut VfsFile) {
    let _drop_me = from_ptr(file);
}

/// Gets the size of the virtual file.
/// Returns true if successful, false otherwise.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn VfsFile_len(file: *mut VfsFile, len: *mut u64) -> bool {
    forget_rust_error();
    let file = unsafe_mut(file);
    let len = unsafe_mut(len);
    match file.len() {
        Err(err) => {
            remember_rust_error(format!("VfsFile_len: {}", err));
            *len = 0;
        }
        Ok(n) => *len = n,
    }
    no_rust_error()
}

/// Reads data from the virtual file.
/// Returns true if successful, false otherwise.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn VfsFile_readExact(file: *mut VfsFile, buffer: *mut u8, length: usize) -> bool {
    forget_rust_error();
    let file = unsafe_mut(file);
    let buffer = unsafe_slice_mut(buffer, length);
    if let Err(err) = file.read_exact(buffer) {
        remember_rust_error(format!("VfsFile_readExact {} {}: {}", file, length, err));
    }
    no_rust_error()
}

/// Reads data from the virtual file might return less bytes than requested
/// Returns the number of bytes read or ´usize::MAX´ if there is an error.
/// Sets the rust error.
#[no_mangle]
pub unsafe extern "C" fn VfsFile_read(file: *mut VfsFile, buffer: *mut u8, length: usize) -> usize {
    forget_rust_error();
    let file = unsafe_mut(file);
    let buffer = unsafe_slice_mut(buffer, length);
    match file.read(buffer) {
        Ok(b) => {
            return b;
        },
        Err(err) => {
            remember_rust_error(format!("VfsFile_readExact {} {}: {}", file, length, err));
            return usize::MAX;
        }
    };
}

/// Seeks to an offset relative to the start of the virtual file.
/// Returns true if successful, false otherwise.
/// Sets the rust error.
/// `out_position` is optional.
#[no_mangle]
pub extern "C" fn VfsFile_seekFromStart(
    file: *mut VfsFile,
    offset: u64,
    out_position: *mut u64,
) -> bool {
    forget_rust_error();
    let file = unsafe_mut(file);
    let out_position = unsafe_mut_option(out_position);
    match file.seek(SeekFrom::Start(offset)) {
        Err(err) => {
            remember_rust_error(format!("VfsFile_seekFromStart {}: {}", offset, err));
        }
        Ok(position) => {
            if let Some(out) = out_position {
                *out = position;
            }
        }
    }
    no_rust_error()
}

/// Seeks to an offset relative to the end of the virtual file.
/// Returns true if successful, false otherwise.
/// Sets the rust error.
/// `out_position` is optional.
#[no_mangle]
pub extern "C" fn VfsFile_seekFromEnd(
    file: *mut VfsFile,
    offset: i64,
    out_position: *mut u64,
) -> bool {
    forget_rust_error();
    let file = unsafe_mut(file);
    let out_position = unsafe_mut_option(out_position);
    match file.seek(SeekFrom::End(offset)) {
        Err(err) => {
            remember_rust_error(format!("VfsFile_seekFromEnd {}: {}", offset, err));
        }
        Ok(position) => {
            if let Some(out) = out_position {
                *out = position;
            }
        }
    }
    no_rust_error()
}

/// Seeks to an offset relative to the current position in the virtual file.
/// Returns true if successful, false otherwise.
/// Sets the rust error.
/// `out_position` is optional.
#[no_mangle]
pub extern "C" fn VfsFile_seekFromCurrent(
    file: *mut VfsFile,
    offset: i64,
    out_position: *mut u64,
) -> bool {
    forget_rust_error();
    let file = unsafe_mut(file);
    let out_position = unsafe_mut_option(out_position);
    match file.seek(SeekFrom::Current(offset)) {
        Err(err) => {
            remember_rust_error(format!("VfsFile_seekFromCurrent {}: {}", offset, err));
        }
        Ok(position) => {
            if let Some(out) = out_position {
                *out = position;
            }
        }
    }
    no_rust_error()
}

/// Writes data from the buffer to the virtual file.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn VfsFile_writeAll(file: *mut VfsFile, buffer: *const u8, length: usize) -> bool {
    forget_rust_error();
    let file = unsafe_mut(file);
    let buffer = unsafe_slice(buffer, length);
    while let Err(err) = file.write_all(buffer) {
        remember_rust_error(format!("VfsFile_writeAll {}: {}", length, err));
    }
    no_rust_error()
}
