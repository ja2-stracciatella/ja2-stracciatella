//! This module contains a class that encodes known path types with [percent-encoding].
//!
//! Paths that are unicode without '\0' or '%' will remain unchanged.
//! Compatibility with `String` and `CString` is always maintained.
//!
//! Unix and mac paths are `[u8]`.
//! Windows paths are `[u16]`.
//! Fltk paths are utf8.
//!
//! # Format:
//!  * as much data as possible is kept as unencoded utf8
//!  * '\0' and '%' are always encoded
//!  * `u8` data and ASCII `u16` data is encoded as "%XX"
//!  * `u16` data is encoded as "%uXXXX"
//!
//! [percent-encoding]: https://en.wikipedia.org/wiki/Percent-encoding

use std::char;
use std::convert::TryFrom;
use std::ffi::{CStr, CString, OsStr};
use std::fmt;
use std::path::{Path, PathBuf};
use std::str;

/// Unicode string that is compatible with `CString`.
/// It can encode `Path`, `[u8]` and `[u16]`.
#[derive(Clone, Default, Hash, Eq, PartialEq, Ord, PartialOrd)]
pub struct AnyPath {
    inner: String,
}

impl AnyPath {
    /// Creates a `AnyPath` that is empty.
    pub fn new() -> Self {
        Self::default()
    }

    /// Encodes a `[u8]` path.
    pub fn encode_slice_u8(path: &[u8]) -> Self {
        let mut any_path = Self::default();
        any_path.push_slice_u8(path);
        any_path
    }

    /// Encodes a `[u16]` path.
    pub fn encode_slice_u16(path: &[u16]) -> Self {
        let mut any_path = Self::default();
        any_path.push_slice_u16(path);
        any_path
    }

    /// Encodes a `Path`.
    pub fn encode_path(path: &Path) -> Self {
        let mut any_path = Self::default();
        any_path.push_os_str(path.as_os_str());
        any_path
    }

    /// Decodes a `Vec<u8>` if possible.
    pub fn decode_vec_u8(&self) -> Result<Vec<u8>, String> {
        let mut vec = Vec::new();
        for split in percent_split(self.inner.as_str())? {
            match split {
                PercentSplit::U16(_) => {
                    return Err("u16 data is not supported".into());
                }
                PercentSplit::U8(b) => vec.push(b),
                PercentSplit::Str(s) => s.as_bytes().iter().for_each(|&b| vec.push(b)),
            }
        }
        Ok(vec)
    }

    /// Decodes a `Vec<u16>` if possible.
    pub fn decode_vec_u16(&self) -> Result<Vec<u16>, String> {
        let mut vec = Vec::new();
        for split in percent_split(self.inner.as_str())? {
            match split {
                PercentSplit::U16(w) => vec.push(w),
                PercentSplit::U8(b) => {
                    if b < 0x80 {
                        vec.push(u16::from(b));
                    } else {
                        return Err("u8 data must be ASCII".into());
                    }
                }
                PercentSplit::Str(s) => s.encode_utf16().for_each(|w| vec.push(w)),
            }
        }
        Ok(vec)
    }

    /// Decodes a `PathBuf` if possible.
    pub fn decode_path_buf(&self) -> Result<PathBuf, String> {
        #[cfg(unix)]
        {
            use std::os::unix::ffi::OsStrExt;

            let vec = self.decode_vec_u8()?;
            Ok(OsStr::from_bytes(vec.as_ref()).to_owned().into())
        }
        #[cfg(windows)]
        {
            use std::ffi::OsString;
            use std::os::windows::ffi::OsStringExt;

            let vec = self.decode_vec_u16()?;
            Ok(OsString::from_wide(vec.as_ref()).into())
        }
    }

    /// Encodes and adds a u8 value.
    pub fn push_byte(&mut self, b: u8) {
        self.inner.push_str(&format!("%{:02X}", b));
    }

    /// Encodes and adds a wide value.
    pub fn push_wide(&mut self, w: u16) {
        if w < 0x80 {
            self.push_byte(w as u8);
        } else {
            self.inner.push_str(&format!("%u{:04X}", w));
        }
    }

    /// Encodes and adds a character.
    pub fn push_char(&mut self, c: char) {
        if c == '\0' || c == '%' {
            debug_assert!(c.len_utf8() == 1);
            self.push_byte(c as u8);
        } else {
            self.inner.push(c);
        }
    }

    /// Encodes and adds a `[u8]`.
    pub fn push_slice_u8(&mut self, mut data: &[u8]) {
        while !data.is_empty() {
            let s = match str::from_utf8(data) {
                Ok(s) => s,
                Err(err) => {
                    let len = err.valid_up_to();
                    if len == 0 {
                        let len = err.error_len().unwrap_or(data.len());
                        let (invalid, next) = data.split_at(len);
                        data = next;
                        invalid.iter().for_each(|&b| self.push_byte(b));
                        continue;
                    }
                    unsafe { str::from_utf8_unchecked(&data[..len]) }
                }
            };
            data = &data[s.len()..];
            self.push_str(s);
        }
    }

    /// Encodes and adds a `[u8]`.
    pub fn push_slice_u16(&mut self, data: &[u16]) {
        for char_result in char::decode_utf16(data.iter().copied()) {
            match char_result {
                Ok(c) => self.push_char(c),
                Err(err) => {
                    self.push_wide(err.unpaired_surrogate());
                }
            }
        }
    }

    /// Encodes and adds a `str`.
    pub fn push_str(&mut self, s: &str) {
        s.chars().for_each(|c| self.push_char(c));
    }

    /// Encodes and adds an `OsStr`.
    pub fn push_os_str(&mut self, os_s: &OsStr) {
        #[cfg(unix)]
        {
            use std::os::unix::ffi::OsStrExt;

            self.push_slice_u8(os_s.as_bytes());
        }
        #[cfg(windows)]
        {
            use std::os::windows::ffi::OsStrExt;

            let vec: Vec<_> = os_s.encode_wide().collect();
            self.push_slice_u16(&vec);
        }
    }

    /// Returns true if it can be decoded to a `Vec<u8>`.
    pub fn is_vec_u8(&self) -> bool {
        percent_split(self.inner.as_str())
            .expect("percent_split")
            .iter()
            .all(|x| !matches!(x, PercentSplit::U16(_)))
    }

    /// Returns true if it can be decoded to a `Vec<u16>`.
    pub fn is_vec_u16(&self) -> bool {
        percent_split(self.inner.as_str())
            .expect("percent_split")
            .iter()
            .all(|x| match x {
                PercentSplit::U8(b) => *b < 0x80,
                _ => true,
            })
    }

    /// Returns true if it can be decoded to a `PathBuf`.
    pub fn is_path_buf(&self) -> bool {
        #[cfg(unix)]
        {
            self.is_unix()
        }
        #[cfg(windows)]
        {
            self.is_windows()
        }
    }

    /// Returns true if it can be decoded to a unix `PathBuf`.
    pub fn is_unix(&self) -> bool {
        self.is_vec_u8()
    }

    /// Returns true if it can be decoded to windows `PathBuf`.
    pub fn is_windows(&self) -> bool {
        self.is_vec_u16()
    }
}

/// `AnyPath` is always a valid `CString`
impl From<AnyPath> for CString {
    fn from(path: AnyPath) -> Self {
        CString::new(path.inner.into_bytes()).expect("AnyPath does not have nul bytes")
    }
}

/// `AnyPath` is always a valid `String`
impl From<AnyPath> for String {
    fn from(path: AnyPath) -> Self {
        path.inner
    }
}

/// `CString` can be a valid `AnyPath`.
impl TryFrom<CString> for AnyPath {
    type Error = String;

    fn try_from(path: CString) -> Result<Self, String> {
        let inner = path.into_string().map_err(|err| format!("{}", err))?;
        percent_split(&inner)?;
        Ok(Self { inner })
    }
}

/// `&CStr` can be a valid `AnyPath`.
impl TryFrom<&CStr> for AnyPath {
    type Error = String;

    fn try_from(path: &CStr) -> Result<Self, String> {
        Self::try_from(path.to_owned())
    }
}

/// `String` can be a valid `AnyPath`.
impl TryFrom<String> for AnyPath {
    type Error = String;

    fn try_from(path: String) -> Result<Self, String> {
        percent_split(&path)?;
        Ok(Self { inner: path })
    }
}

/// `&str` can be a valid `AnyPath`.
impl TryFrom<&str> for AnyPath {
    type Error = String;

    fn try_from(path: &str) -> Result<Self, String> {
        percent_split(path)?;
        let inner = path.to_owned();
        Ok(Self { inner })
    }
}

/// Inherits all the methods of `str`.
impl AsRef<str> for AnyPath {
    fn as_ref(&self) -> &str {
        self.inner.as_str()
    }
}

/// Matches the inner string.
impl fmt::Display for AnyPath {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let display: &dyn fmt::Display = &self.inner;
        display.fmt(f)
    }
}

/// Matches the inner string.
impl fmt::Debug for AnyPath {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let debug: &dyn fmt::Debug = &self.inner;
        debug.fmt(f)
    }
}

enum PercentSplit<'a> {
    U16(u16),
    U8(u8),
    Str(&'a str),
}

fn percent_split(mut s: &str) -> Result<Vec<PercentSplit>, String> {
    if s.bytes().any(|b| b == 0) {
        return Err("unexpected embedded nul".into());
    }
    let mut splits = Vec::new();
    fn n_chars_len(n: usize, s: &str) -> Option<usize> {
        let mut len = 0;
        let mut chars = s.chars();
        for _ in 0..n {
            if let Some(c) = chars.next() {
                len += c.len_utf8();
            } else {
                return None;
            }
        }
        Some(len)
    }
    while !s.is_empty() {
        if s.starts_with("%u") {
            if let Some(len) = n_chars_len(6, s) {
                let (encoded, next) = s.split_at(len);
                if let Ok(hex_bytes) = hex::decode(&encoded[2..]) {
                    assert!(hex_bytes.len() == 2);
                    let w = (u16::from(hex_bytes[0]) << 8) + u16::from(hex_bytes[1]);
                    splits.push(PercentSplit::U16(w));
                    s = next;
                    continue;
                }
            }
            return Err(format!("expected '%uXXXX', got {:?}", s));
        } else if s.starts_with('%') {
            if let Some(len) = n_chars_len(3, s) {
                let (encoded, next) = s.split_at(len);
                if let Ok(hex_bytes) = hex::decode(&encoded[1..]) {
                    assert!(hex_bytes.len() == 1);
                    let b = hex_bytes[0];
                    splits.push(PercentSplit::U8(b));
                    s = next;
                    continue;
                }
            }
            return Err(format!("expected '%XX', got {:?}", s));
        } else {
            let index = s.find('%').unwrap_or(s.len());
            let (raw, next) = s.split_at(index);
            s = next;
            splits.push(PercentSplit::Str(raw));
        }
    }
    Ok(splits)
}

#[cfg(test)]
mod test {
    use std::convert::TryFrom;
    use std::ffi::CString;
    use std::path::PathBuf;

    use crate::any_path::AnyPath;

    #[test]
    fn try_from_str_ok() {
        fn t(s: &str) {
            assert!(AnyPath::try_from(s).is_ok());
        }
        t("%00"); // nul
        t("%25"); // percent
        t("x"); // 1-byte utf8
        t("\u{00A2}"); // 2-byte utf8
        t("\u{20AC}"); // 3-byte utf8
        t("\u{24B62}"); // 4-byte utf8
        t("%80%C0%FF"); // unix OsString
        t("%uDF62%uD852"); // windows OsString
    }

    #[test]
    fn try_from_str_err() {
        fn t(s: &str) {
            assert!(AnyPath::try_from(s).is_err());
        }
        t("\0");
        t("%");
        t("%F");
        t("%FG");
        t("%u0");
        t("%u00");
        t("%u00F");
        t("%u00FG");
    }

    #[test]
    fn roundtrip_u8_path() {
        fn t(s: &str) {
            let bytes = s.as_bytes();
            assert_eq!(
                AnyPath::encode_slice_u8(bytes).decode_vec_u8(),
                Ok(bytes.to_owned())
            );
        }
        t("\0");
        t("%");
        t("x"); // 1-byte utf8
        t("\u{00A2}"); // 2-byte utf8
        t("\u{20AC}"); // 3-byte utf8
        t("\u{24B62}"); // 4-byte utf8
    }

    #[test]
    fn roundtrip_c_string() {
        fn t(s: &str) {
            let c_path = AnyPath::try_from(s).unwrap();
            let c_string = CString::from(c_path.clone());
            assert_eq!(AnyPath::try_from(c_string.clone()), Ok(c_path.clone())); // CString to AnyPath
            assert_eq!(AnyPath::try_from(c_string.as_ref()), Ok(c_path.clone())); // CStr to AnyPath
            assert_eq!(CString::from(c_path), c_string); // AnyPath to CString
        }
        t("%00"); // null
        t("%25"); // percent
        t("x"); // 1-byte utf8
        t("\u{00A2}"); // 2-byte utf8
        t("\u{20AC}"); // 3-byte utf8
        t("\u{24B62}"); // 4-byte utf8
        t("%80%C0%FF"); // invalid utf8 (unix)
        t("x%uD852x"); // lead surrogate (windows)
        t("x%uDF62x"); // tail surrogate (windows)
        t("x%uDF62%uD852x"); // tail surrogate + lead surrogate (windows)
        t("x%uD852%uD852x"); // lead surrogate + lead surrogate (windows)
    }

    #[test]
    fn roundtrip_path() {
        #[cfg(unix)]
        {
            use std::ffi::OsStr;
            use std::os::unix::ffi::OsStrExt;

            fn t(bytes: &[u8], s: &str) {
                let c_path = AnyPath::try_from(s).unwrap();
                let path_buf = PathBuf::from(OsStr::from_bytes(bytes).to_owned());
                assert_eq!(AnyPath::encode_path(&path_buf), c_path); // Path to AnyPath
                assert_eq!(c_path.decode_path_buf(), Ok(path_buf)); // AnyPath to PathBuf
            }
            t(&[0x00], "%00"); // nul
            t(&[b'%'], "%25"); // percent
            t(&[b'x'], "x"); // 1-byte utf8
            t(&[0xC2, 0xA2], "\u{00A2}"); // 2-byte utf8
            t(&[0xE2, 0x82, 0xAC], "\u{20AC}"); // 3-byte utf8
            t(&[0xF0, 0xA4, 0xAD, 0xA2], "\u{24B62}"); // 4-byte utf8

            // unexpected continuation utf8 = 0x80 to 0xBF
            // lonely start utf8 = 0xC0 to 0xFD
            // impossible utf8 = 0xFE to 0xFF
            for b in 0x80..=0xFF {
                t(&[b], &format!("%{:02X}", b));
            }
        }
        #[cfg(windows)]
        {
            use std::ffi::OsString;
            use std::os::windows::ffi::OsStringExt;

            fn t(wide: &[u16], s: &str) {
                let c_path = AnyPath::try_from(s).unwrap();
                let path_buf = PathBuf::from(OsString::from_wide(wide));
                assert_eq!(AnyPath::encode_path(&path_buf), c_path); // Path to AnyPath
                assert_eq!(c_path.decode_path_buf(), Ok(path_buf)); // AnyPath to PathBuf
            }
            t(&[0x0000], "%00"); // nul
            t(&[u16::from(b'%')], "%25"); // percent
            t(&[u16::from(b'x')], "x");
            t(&[0xD852], "%uD852"); // lead surrogate
            t(&[0xDF62], "%uDF62"); // tail surrogate
            t(&[0xDF62, 0xD852], "%uDF62%uD852"); // tail surrogate + lead surrogate
            t(&[0xD852, 0xD852], "%uD852%uD852"); // lead surrogate + lead surrogate
            t(&[0xD852, 0xDF62], "\u{24B62}"); // lead surrogate + tail surrogate
        }
    }

    #[test]
    fn decode_path_err() {
        fn t(s: &str) {
            let c_path = AnyPath::try_from(s).unwrap();
            assert!(c_path.decode_path_buf().is_err());
        }
        t("%uDF62%uD852%80%C0%FF"); // must be either unix or windows
        #[cfg(unix)]
        {
            t("%uDF62%uD852"); // wide data is not supported in unix
        }
        #[cfg(windows)]
        {
            t("%80%C0%FF"); // byte data must be ASCII in windows
        }
    }

    #[test]
    fn is_unix_is_windows() {
        fn t(s: &str, unix: bool, windows: bool) {
            let c_path = AnyPath::try_from(s).unwrap();
            assert_eq!(c_path.is_unix(), unix);
            assert_eq!(c_path.is_windows(), windows);
        }
        t("x%00x", true, true); // nul
        t("x%25x", true, true); // percent
        t("x", true, true); // 1-byte utf8
        t("\u{00A2}", true, true); // 2-byte utf8
        t("\u{20AC}", true, true); // 3-byte utf8
        t("\u{24B62}", true, true); // 4-byte utf8
        t("%80%C0%FF", true, false); // unix Path
        t("%uDF62%uD852", false, true); // windows Path
        t("%80%C0%FF%uDF62%uD852", false, false); // mixed
    }
}
