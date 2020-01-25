//! This module contains the C interface for [`std::path`], excluding filesystem interactions.
//!
//! [`std::path`]: https://doc.rust-lang.org/std/path/index.html

use std::ptr;

use crate::c::common::*;

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
