//! This module contains the C interface for [`std::path`], excluding filesystem interactions.
//!
//! [`std::path`]: https://doc.rust-lang.org/std/path/index.html

use std::ptr;

use crate::c::common::*;

/// Gets the extension of the path.
/// Returns null if there is no extension.
#[no_mangle]
pub extern "C" fn Path_extension(path: *const c_char) -> *mut c_char {
    let path = path_from_c_str_or_panic(unsafe_c_str(path));
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
    let path = path_from_c_str_or_panic(unsafe_c_str(path));
    if let Some(filename) = path.file_name() {
        let c_filename = c_string_from_path_or_panic(filename.as_ref());
        c_filename.into_raw()
    } else {
        ptr::null_mut()
    }
}

/// Gets the parent path of the path.
/// Returns null if there is no parent path.
#[no_mangle]
pub extern "C" fn Path_parent(path: *const c_char) -> *mut c_char {
    let path = path_from_c_str_or_panic(unsafe_c_str(path));
    if let Some(parent) = path.parent() {
        let c_parent = c_string_from_path_or_panic(parent);
        c_parent.into_raw()
    } else {
        ptr::null_mut()
    }
}
