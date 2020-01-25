//! This module contains the C interface for [`stracciatella::fs`].
//!
//! [`stracciatella::fs`]: ../../stracciatella/fs/index.html

use stracciatella::fs;

use crate::c::common::*;

/// Checks if the path exists.
#[no_mangle]
pub extern "C" fn Fs_exists(path: *const c_char) -> bool {
    let path = path_from_c_str_or_panic(unsafe_c_str(path));
    fs::metadata(&path).is_ok()
}

/// Renames a file or directory.
/// Sets the rust error.
/// @see https://doc.rust-lang.org/std/fs/fn.rename.html
#[no_mangle]
pub extern "C" fn Fs_rename(from: *const c_char, to: *const c_char) -> bool {
    forget_rust_error();
    let from = path_from_c_str_or_panic(unsafe_c_str(from));
    let to = path_from_c_str_or_panic(unsafe_c_str(to));
    if let Err(err) = fs::rename(&from, &to) {
        remember_rust_error(format!("Fs_rename {:?} {:?}: {}", from, to, err));
    }
    no_rust_error()
}
