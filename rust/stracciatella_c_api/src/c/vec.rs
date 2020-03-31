//! This module contains wrappers around [`Vec<T>`].
//!
//! [`Vec<T>`]: https://doc.rust-lang.org/std/vec/struct.Vec.html

use std::ffi::CString;

use crate::c::common::*;

/// A wrapper around `Vec<CString>` for C.
#[derive(Default)]
pub struct VecCString {
    pub inner: Vec<CString>,
}

impl VecCString {
    pub fn new() -> Self {
        Self { inner: Vec::new() }
    }
    pub fn from_vec(vec: Vec<CString>) -> Self {
        Self { inner: vec }
    }
}

/// Deletes the vector.
#[no_mangle]
pub extern "C" fn VecCString_destroy(vec: *mut VecCString) {
    let _drop_me = from_ptr(vec);
}

/// Returns the vector length.
#[no_mangle]
pub extern "C" fn VecCString_length(vec: *mut VecCString) -> size_t {
    let vec = unsafe_ref(vec);
    vec.inner.len()
}

/// Returns the string at the target index.
/// The caller is responsible for the returned memory.
#[no_mangle]
pub extern "C" fn VecCString_get(vec: *mut VecCString, index: size_t) -> *mut c_char {
    let vec = unsafe_ref(vec);
    vec.inner[index].clone().into_raw()
}
