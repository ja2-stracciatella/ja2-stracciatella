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
#[unsafe(no_mangle)]
pub extern "C" fn VecCString_create() -> *mut VecCString {
    let vec = VecCString::default();
    into_ptr(vec)
}

/// Destroys the vector.
/// coverity[+free : arg-0]
#[unsafe(no_mangle)]
pub extern "C" fn VecCString_destroy(vec: *mut VecCString) {
    let _drop_me = from_ptr(vec);
}

/// Returns the length of the vector.
#[unsafe(no_mangle)]
pub extern "C" fn VecCString_len(vec: *mut VecCString) -> usize {
    let vec = unsafe_ref(vec);
    vec.inner.len()
}

/// Returns the string value at the vector index.
#[unsafe(no_mangle)]
pub extern "C" fn VecCString_get(vec: *mut VecCString, index: usize) -> *mut c_char {
    let vec = unsafe_ref(vec);
    vec.inner[index].clone().into_raw()
}

/// Adds a string value to the end of the vector.
#[unsafe(no_mangle)]
pub extern "C" fn VecCString_push(vec: *mut VecCString, value: *const c_char) {
    let vec = unsafe_mut(vec);
    let value = unsafe_c_str(value);
    vec.inner.push(value.to_owned())
}

/// A wrapper around `Vec<u8>` for C.
#[derive(Default)]
pub struct VecU8 {
    inner: Vec<u8>,
}

impl From<Vec<u8>> for VecU8 {
    fn from(vec: Vec<u8>) -> Self {
        Self { inner: vec }
    }
}

/// Creates an empty vector.
/// coverity[+alloc]
#[unsafe(no_mangle)]
pub extern "C" fn VecU8_create() -> *mut VecU8 {
    let vec = VecU8::default();
    into_ptr(vec)
}

/// Destroys the vector.
/// coverity[+free : arg-0]
#[unsafe(no_mangle)]
pub extern "C" fn VecU8_destroy(vec: *mut VecU8) {
    let _drop_me = from_ptr(vec);
}

/// Returns the raw pointer to the vector data.
#[unsafe(no_mangle)]
pub extern "C" fn VecU8_as_ptr(vec: *mut VecU8) -> *const u8 {
    let vec = unsafe_ref(vec);
    vec.inner.as_ptr()
}

/// Returns the length of the vector.
#[unsafe(no_mangle)]
pub extern "C" fn VecU8_len(vec: *mut VecU8) -> usize {
    let vec = unsafe_ref(vec);
    vec.inner.len()
}

/// Returns the `u8` value at the vector index.
#[unsafe(no_mangle)]
pub extern "C" fn VecU8_get(vec: *mut VecU8, index: usize) -> u8 {
    let vec = unsafe_ref(vec);
    vec.inner[index]
}

/// Adds a `u8` value to the end of the vector.
#[unsafe(no_mangle)]
pub extern "C" fn VecU8_push(vec: *mut VecU8, value: u8) {
    let vec = unsafe_mut(vec);
    vec.inner.push(value)
}

/// A wrapper around `Vec<usize>` for C.
#[derive(Default)]
pub struct VecUSize {
    inner: Vec<usize>,
}

impl From<Vec<usize>> for VecUSize {
    fn from(vec: Vec<usize>) -> Self {
        Self { inner: vec }
    }
}

/// Creates an empty vector.
/// coverity[+alloc]
#[unsafe(no_mangle)]
pub extern "C" fn VecUSize_create() -> *mut VecUSize {
    let vec = VecUSize::default();
    into_ptr(vec)
}

/// Destroys the vector.
/// coverity[+free : arg-0]
#[unsafe(no_mangle)]
pub extern "C" fn VecUSize_destroy(vec: *mut VecUSize) {
    let _drop_me = from_ptr(vec);
}

/// Returns the raw pointer to the vector data.
#[unsafe(no_mangle)]
pub extern "C" fn VecUSize_as_ptr(vec: *mut VecUSize) -> *const usize {
    let vec = unsafe_ref(vec);
    vec.inner.as_ptr()
}

/// Returns the length of the vector.
#[unsafe(no_mangle)]
pub extern "C" fn VecUSize_len(vec: *mut VecUSize) -> usize {
    let vec = unsafe_ref(vec);
    vec.inner.len()
}

/// Returns the `usize` value at the vector index.
#[unsafe(no_mangle)]
pub extern "C" fn VecUSize_get(vec: *mut VecUSize, index: usize) -> usize {
    let vec = unsafe_ref(vec);
    vec.inner[index]
}

/// Adds a `usize` value to the end of the vector.
#[unsafe(no_mangle)]
pub extern "C" fn VecUSize_push(vec: *mut VecUSize, value: usize) {
    let vec = unsafe_mut(vec);
    vec.inner.push(value)
}
