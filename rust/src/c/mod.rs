//! This module and it's submodules contains code for C.

pub mod librarydb;

pub mod error {
    //! This module contains error handling code for C.

    use crate::c::common::*;

    /// Returns a copy of the thread local rust error or null.
    /// The caller is responsible for the memory.
    #[no_mangle]
    pub extern "C" fn get_rust_error() -> *mut c_char {
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
    #![allow(dead_code)]

    pub use libc::{c_char, c_int, size_t};
    use std::cell::RefCell;
    use std::ffi::{CStr, CString};
    use std::path::Path;
    use std::slice;

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

    /// Converts a CStr to a Path. Panics on failure.
    pub fn path_from_c_str_or_panic(c_str: &CStr) -> &Path {
        let bytes = c_str.to_bytes();
        #[cfg(unix)]
        {
            use std::os::unix::ffi::OsStrExt;
            Path::new(std::ffi::OsStr::from_bytes(&bytes))
        }
        #[cfg(not(unix))]
        {
            match std::str::from_utf8(&bytes) {
                Ok(s) => Path::new(s),
                Err(e) => panic!("Converting {:?} to Path: {:?}", &c_str, e),
            }
        }
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
    use crate::c::common::*;
    use crate::c::error::*;
    use crate::free_rust_string;
    use std::cell::RefCell;
    use std::ffi::{CStr, CString};
    use std::path::Path;

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
        assert_eq!(path_from_c_str_or_panic(&c_str), Path::new("123"));
        assert_eq!(c_string_from_str("123").to_bytes(), b"123");
        assert_eq!(c_string_from_str("123\0nope").to_bytes(), b"123");
        #[cfg(unix)]
        {
            // Path supports invalid utf8 on unix
            let c_str = CStr::from_bytes_with_nul(b"123\xe1\0").unwrap();
            assert_eq!(path_from_c_str_or_panic(&c_str).as_os_str().len(), 4);
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
        if let Ok(c_str) = CStr::from_bytes_with_nul(b"123\xe1\0") {
            // panics, Path only supports invalid utf8 on unix
            path_from_c_str_or_panic(&c_str);
        }
    }

    #[test]
    fn test_rust_error() {
        fn error() -> Option<CString> {
            let ptr = get_rust_error(); // get copy
            if ptr.is_null() {
                None
            } else {
                let c_string = unsafe_c_str(ptr).to_owned();
                free_rust_string(ptr); // free copy
                Some(c_string)
            }
        }
        remember_rust_error("rust error"); // set
        assert_eq!(error(), Some(CString::new("rust error").unwrap()));
        forget_rust_error(); // unset
        assert_eq!(error(), None);
    }
}
