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

impl From<Vec<CString>> for VecCString {
    fn from(vec: Vec<CString>) -> Self {
        Self { inner: vec }
    }
}

/// Creates an empty vector.
/// coverity[+alloc]
#[no_mangle]
pub extern "C" fn VecCString_create() -> *mut VecCString {
    let vec = VecCString::default();
    into_ptr(vec)
}

/// Destroys the vector.
/// coverity[+free : arg-0]
#[no_mangle]
pub extern "C" fn VecCString_destroy(vec: *mut VecCString) {
    let _drop_me = from_ptr(vec);
}

/// Returns the length of the vector.
#[no_mangle]
pub extern "C" fn VecCString_len(vec: *mut VecCString) -> usize {
    let vec = unsafe_ref(vec);
    vec.inner.len()
}

/// Returns the string value at the vector index.
#[no_mangle]
pub extern "C" fn VecCString_get(vec: *mut VecCString, index: usize) -> *mut c_char {
    let vec = unsafe_ref(vec);
    vec.inner[index].clone().into_raw()
}

/// Adds a string value to the end of the vector.
#[no_mangle]
pub extern "C" fn VecCString_push(vec: *mut VecCString, value: *const c_char) {
    let vec = unsafe_mut(vec);
    let value = unsafe_c_str(value);
    vec.inner.push(value.to_owned())
}
