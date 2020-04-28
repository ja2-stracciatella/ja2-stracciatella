//! This module contains the C interface for [`std::path`], excluding filesystem interactions.
//!
//! [`std::path`]: https://doc.rust-lang.org/std/path/index.html

use std::convert::TryFrom;
use std::ffi::CString;
use std::ptr;
use std::usize;

use crate::any_path::AnyPath;
use crate::c::common::*;

/// Encodes a `[u8]` path.
/// Returns the encoded path.
#[no_mangle]
pub extern "C" fn Path_encodeU8(path: *const u8, path_len: usize) -> *mut c_char {
    let path = unsafe_slice(path, path_len);
    CString::from(AnyPath::encode_slice_u8(path)).into_raw()
}

/// Decodes a `[u8]` path to the buffer.
/// The decoded path is never bigger than the encoded path.
/// Returns the amount of decoded bytes, or `usize::MAX` if there is an error.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Path_decodeU8(path: *const c_char, buf: *mut u8, buf_len: usize) -> usize {
    forget_rust_error();
    let path = unsafe_c_str(path);
    let buf = unsafe_slice_mut(buf, buf_len);
    let vec_result = AnyPath::try_from(path).and_then(|x| x.decode_vec_u8());
    match vec_result {
        Err(err) => {
            remember_rust_error(format!("Path_decodeU8 {:?} {}: {}", path, buf_len, err));
            usize::MAX
        }
        Ok(vec) => {
            let n = vec.len().min(buf_len);
            buf[..n].copy_from_slice(&vec[..n]);
            for b in buf.iter_mut().skip(n) {
                *b = 0;
            }
            vec.len()
        }
    }
}

/// Gets the extension of the path.
/// Returns null if there is no extension.
#[no_mangle]
pub extern "C" fn Path_extension(path: *const c_char) -> *mut c_char {
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    if let Some(extension) = path.extension() {
        let c_extension = c_string_from_path_or_panic(extension.as_ref());
        c_extension.into_raw()
    } else {
        ptr::null_mut()
    }
}

/// Gets the filename of the path.
/// Returns null if there is no filename.
#[no_mangle]
pub extern "C" fn Path_filename(path: *const c_char) -> *mut c_char {
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    if let Some(filename) = path.file_name() {
        let c_filename = c_string_from_path_or_panic(filename.as_ref());
        c_filename.into_raw()
    } else {
        ptr::null_mut()
    }
}

/// Extends the path.
/// If `newpath` is absolute, it replaces the current path.
#[no_mangle]
pub extern "C" fn Path_push(path: *const c_char, newpath: *const c_char) -> *mut c_char {
    let mut path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let newpath = path_buf_from_c_str_or_panic(unsafe_c_str(newpath));
    path.push(&newpath);
    c_string_from_path_or_panic(&path).into_raw()
}

/// Checks if the path is absolute.
#[no_mangle]
pub extern "C" fn Path_isAbsolute(path: *const c_char) -> bool {
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    path.is_absolute()
}

/// Gets the parent path of the path.
/// Returns null if there is no parent path.
#[no_mangle]
pub extern "C" fn Path_parent(path: *const c_char) -> *mut c_char {
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    if let Some(parent) = path.parent() {
        let c_parent = c_string_from_path_or_panic(parent);
        c_parent.into_raw()
    } else {
        ptr::null_mut()
    }
}

/// Sets the extension of the path.
/// @see https://doc.rust-lang.org/std/path/struct.PathBuf.html#method.set_extension
#[no_mangle]
pub extern "C" fn Path_setExtension(path: *const c_char, extension: *const c_char) -> *mut c_char {
    let mut path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let extension = path_buf_from_c_str_or_panic(unsafe_c_str(extension));
    path.set_extension(extension.as_os_str());
    let new_path = c_string_from_path_or_panic(&path);
    new_path.into_raw()
}

/// Sets the filename of the path.
#[no_mangle]
pub extern "C" fn Path_setFilename(path: *const c_char, filename: *const c_char) -> *mut c_char {
    let mut path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let filename = path_buf_from_c_str_or_panic(unsafe_c_str(filename));
    path.set_file_name(filename.as_os_str());
    let new_path = c_string_from_path_or_panic(&path);
    new_path.into_raw()
}
