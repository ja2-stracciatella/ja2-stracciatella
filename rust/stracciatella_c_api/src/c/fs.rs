//! This module contains the C interface for [`stracciatella::fs`].
//!
//! [`stracciatella::fs`]: ../../stracciatella/fs/index.html

use std::ptr;

use stracciatella::fs;

use crate::c::common::*;
use crate::c::misc::VecCString;

/// Checks if the path exists.
#[no_mangle]
pub extern "C" fn Fs_exists(path: *const c_char) -> bool {
    let path = path_from_c_str_or_panic(unsafe_c_str(path));
    fs::metadata(&path).is_ok()
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
    let dir = path_from_c_str_or_panic(unsafe_c_str(dir));
    match fs::read_dir_paths(&dir, ignore_entry_errors) {
        Err(err) => {
            remember_rust_error(format!("Fs_readDirPaths {:?}: {}", dir, err));
            ptr::null_mut()
        }
        Ok(vec) => {
            let vec = vec
                .iter()
                .map(|x| c_string_from_path_or_panic(&x))
                .collect();
            let c_vec = VecCString::from_vec(vec);
            into_ptr(c_vec)
        }
    }
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
