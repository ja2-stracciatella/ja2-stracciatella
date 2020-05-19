//! This module and it's submodules contains code for C.
//!
//! The C-rust interface should follow the naming style in:
//! http://geosoft.no/development/cppstyle.html

pub mod config;
pub mod fs;
pub mod librarydb;
pub mod logger;
pub mod misc;
pub mod path;
pub mod vec;

pub mod error {
    //! This module contains error handling code for C.

    use crate::c::common::*;

    /// Returns a copy of the thread local rust error or null.
    /// The caller is responsible for the memory.
    #[no_mangle]
    pub extern "C" fn getRustError() -> *mut c_char {
        RUST_ERROR.with(|x| {
            if let Some(ref error) = *x.borrow() {
                return error.clone().into_raw();
            }
            std::ptr::null_mut()
        })
    }
}

pub(crate) mod common {
    //! This module contains common rust code to work with C.
    //!
    //! All pointers that come from C are unsafe.
    //! C can send a pointer that has already been freed.
    //! C can send a pointer that is being used in a different thread.
    //! C can send a pointer that is a different type of data (another struct or a number).
    //! Only null pointers can be detected safely, the rest can't be checked in rust.
    //!
    //! When you declare a function unsafe you automatically get an unsafe body (disables safety checks).
    //! All `pub extern "C"` functions that receive a pointer from C are unsafe.
    //! We want the safety checks so we DO NOT declare them unsafe.
    //! Hopefully in the future there will be a way for unsafe functions to get a safe body.
    //! See https://github.com/rust-lang/rfcs/pull/2585
    #![allow(dead_code)]

    use std::cell::RefCell;
    use std::convert::TryFrom;
    use std::ffi::{CStr, CString};
    use std::path::{Path, PathBuf};
    use std::slice;

    pub use libc::{c_char, c_int, size_t};

    use crate::any_path::AnyPath;

    thread_local!(
        /// A thread local error for C.
        pub static RUST_ERROR: RefCell<Option<CString>> = RefCell::new(None)
    );

    /// Sets the thread local error string for C.
    pub fn remember_rust_error<T: AsRef<str>>(msg: T) {
        RUST_ERROR.with(|x| {
            let mut error = x.borrow_mut();
            *error = Some(c_string_from_str(msg.as_ref()));
        });
    }

    /// Unsets the thread local error string for C.
    pub fn forget_rust_error() {
        RUST_ERROR.with(|x| {
            let mut error = x.borrow_mut();
            *error = None;
        });
    }

    /// Returns true if there is no thread local error string for C, false otherwise.
    pub fn no_rust_error() -> bool {
        RUST_ERROR.with(|x| {
            let error = x.borrow();
            error.is_none()
        })
    }

    /// Moves a value into a wrapped raw pointer. The caller is responsible for the memory.
    pub fn into_ptr<T>(value: T) -> *mut T {
        Box::into_raw(Box::new(value))
    }

    /// Moves the value out of a wrapped raw pointer. The caller is no longer responsible for the memory.
    pub fn from_ptr<T>(ptr: *mut T) -> Box<T> {
        assert!(!ptr.is_null());
        unsafe { Box::from_raw(ptr) }
    }

    /// Gets a mutable reference from a mutable C pointer.
    pub fn unsafe_mut<'a, T>(ptr: *mut T) -> &'a mut T {
        assert!(!ptr.is_null());
        unsafe { &mut *ptr }
    }

    /// Gets a mutable reference from a mutable C pointer.
    pub fn unsafe_mut_option<'a, T>(ptr: *mut T) -> Option<&'a mut T> {
        if ptr.is_null() {
            None
        } else {
            Some(unsafe { &mut *ptr })
        }
    }

    /// Gets a reference from a const C pointer.
    pub fn unsafe_ref<'a, T>(ptr: *const T) -> &'a T {
        assert!(!ptr.is_null());
        unsafe { &*ptr }
    }

    /// Gets a CStr from a const C character pointer.
    pub fn unsafe_c_str<'a>(ptr: *const c_char) -> &'a CStr {
        assert!(!ptr.is_null());
        unsafe { CStr::from_ptr(ptr) }
    }

    /// Gets a slice from a const C pointer and length.
    pub fn unsafe_slice<'a, T>(ptr: *const T, len: usize) -> &'a [T] {
        assert!(!ptr.is_null());
        unsafe { slice::from_raw_parts(ptr, len) }
    }

    /// Gets a mutable slice from a mutable C pointer and length.
    pub fn unsafe_slice_mut<'a, T>(ptr: *mut T, len: usize) -> &'a mut [T] {
        assert!(!ptr.is_null());
        unsafe { slice::from_raw_parts_mut(ptr, len) }
    }

    /// Converts a CStr to a str. Panics on failure.
    pub fn str_from_c_str_or_panic(c_str: &CStr) -> &str {
        match c_str.to_str() {
            Ok(s) => s,
            Err(e) => panic!("Converting {:?} to str: {:?}", &c_str, e),
        }
    }

    /// Converts a CStr to a PathBuf. Panics on failure.
    pub fn path_buf_from_c_str_or_panic(c_str: &CStr) -> PathBuf {
        let c_path = AnyPath::try_from(c_str).expect("AnyPath");
        c_path.decode_path_buf().expect("PathBuf")
    }

    // Converts a Path to a CString. Discards characters starting with the first nul character.
    pub fn c_string_from_path_or_panic(path: &Path) -> CString {
        let c_path = AnyPath::encode_path(path);
        c_path.into()
    }

    /// Converts a str to a CString. Discards characters starting with the first nul character.
    pub fn c_string_from_str(s: &str) -> CString {
        let bytes = match s.find('\0') {
            Some(pos) => s[..pos].as_bytes(),
            None => s.as_bytes(),
        };
        unsafe { CString::from_vec_unchecked(bytes.to_vec()) }
    }
}

#[cfg(test)]
mod tests {
    use std::cell::RefCell;
    use std::ffi::{CStr, CString};
    use std::path::Path;

    use crate::c::common::*;
    use crate::c::error::*;
    use crate::c::misc::CString_destroy;

    #[test]
    fn test_pointers() {
        // struct
        let mut outer: RefCell<u8> = RefCell::new(1);
        struct PointerTest<'a> {
            inner: &'a mut RefCell<u8>,
        }
        impl<'a> Drop for PointerTest<'a> {
            fn drop(&mut self) {
                *self.inner.borrow_mut() = 99;
            }
        }
        let value1;
        let value2;
        let value99;
        // into pointer, must manage the memory
        let ptr = into_ptr(PointerTest { inner: &mut outer });
        {
            // pointer as immutable reference
            value1 = *unsafe_ref(ptr).inner.borrow();
        }
        {
            // pointer as mutable reference
            *unsafe_mut(ptr).inner.borrow_mut() = 2;
        }
        {
            // from pointer, rust manages the memory
            let drop_me = from_ptr(ptr);
            value2 = *drop_me.inner.borrow();
        }
        value99 = *outer.borrow();
        assert_eq!(value1, 1); // immutable
        assert_eq!(value2, 2); // mutable
        assert_eq!(value99, 99); // drop

        // C string
        let data = "123\0nope";
        let ptr = data.as_ptr() as *const c_char;
        assert_eq!(unsafe_c_str(ptr).to_bytes(), b"123");

        // slice
        let mut data = [1, 2];
        assert_eq!(unsafe_slice(data.as_ptr(), 2), data);
        assert_eq!(unsafe_slice_mut(data.as_mut_ptr(), 2), data);
    }

    #[test]
    fn test_conversions() {
        let c_str = CStr::from_bytes_with_nul(b"123\0").unwrap();
        assert_eq!(str_from_c_str_or_panic(&c_str), "123");
        assert_eq!(path_buf_from_c_str_or_panic(&c_str), Path::new("123"));
        assert_eq!(
            c_string_from_path_or_panic(Path::new("123")).to_bytes(),
            b"123"
        );
        assert_eq!(
            c_string_from_path_or_panic(Path::new("123\0yup%")).to_bytes(),
            b"123%00yup%25"
        );
        assert_eq!(c_string_from_str("123").to_bytes(), b"123");
        assert_eq!(c_string_from_str("123\0nope").to_bytes(), b"123");
        #[cfg(unix)]
        {
            // Path supports invalid utf8 on unix
            let c_str = CStr::from_bytes_with_nul(b"123%E1\0").unwrap();
            let path = path_buf_from_c_str_or_panic(&c_str);
            assert_eq!(path.as_os_str().len(), 4);
            assert_eq!(c_string_from_path_or_panic(&path).as_ref(), c_str);
        }
    }

    #[test]
    #[should_panic]
    fn test_str_from_c_str_panic() {
        if let Ok(c_str) = CStr::from_bytes_with_nul(b"123\xe1\0") {
            // panics, str only supports valid utf8
            str_from_c_str_or_panic(&c_str);
        }
    }

    #[test]
    #[should_panic]
    #[cfg(not(unix))]
    fn test_path_from_c_str_panic_not_unix() {
        if let Ok(c_str) = CStr::from_bytes_with_nul(b"123%E1\0") {
            // panics, Path only supports invalid utf8 on unix
            path_buf_from_c_str_or_panic(&c_str);
        }
    }

    #[test]
    fn test_rust_error() {
        fn error() -> Option<CString> {
            let ptr = getRustError(); // get copy
            if ptr.is_null() {
                None
            } else {
                let c_string = unsafe_c_str(ptr).to_owned();
                CString_destroy(ptr); // free copy
                Some(c_string)
            }
        }
        remember_rust_error("rust error"); // set
        assert_eq!(error(), Some(CString::new("rust error").unwrap()));
        forget_rust_error(); // unset
        assert_eq!(error(), None);
    }
}
