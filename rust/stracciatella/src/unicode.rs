//! Correct unicode handling.
//!
//! Correct unicode is hard, I'll spare you the gory details...
//! This module adds a slight runtime speed cost and a considerable binary size cost.
//! Uses the crates [`unicode-normalization`] and [`caseless`].
//!
//! In javascript [`String.prototype.normalize()`] defaults to NFC so we use it too.
//!
//! We use the default `String` order, which is probably not correct unicode order.
//!
//!
//! # Normalization
//!
//! Two unicode strings with different bytes can be considered equal.
//! Normalizing makes them have the same bytes as long as the same normalization form is used.
//!
//! We use [canonical equivalence] to ensure characters have the same visual appearance and behavior.
//!
//! There is a runtime space-time tradeoff between the two canonical [normalization forms]:
//!  * NFD = more string space, less normalization time (decomposition)
//!  * NFC = less string space, more normalization time (decomposition + composition)
//!
//!
//! # Partial strings
//!
//! A concatenated string must be normalized even if the original strings were normalized.
//!
//! I'm not sure, but utf8 substrings are probably normalized if the original was normalized.
//!
//!
//! # Ordering
//!
//! Should depend on the locale.
//!
//!
//! # Case sensitive
//!
//! Compare normalized strings.
//!
//!
//! # Case insensitive
//!
//! Compare caseless normalized strings.
//!
//! To get a caseless string we must normalize, then get a caseless representation with [case folding] and normalize again.
//!
//! Based on [(faq/casemap_charprop.html#2)], you can expect a lowercase result most of the time:
//! "Case folding in Unicode is primarily based on the lowercase mapping..."
//!
//!
//! [`caseless`]: https://crates.io/crates/caseless
//! [`unicode-normalization`]: https://crates.io/crates/unicode-normalization
//! [`String.prototype.normalize()`]: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String/normalize
//! [canonical equivalence]: http://www.unicode.org/reports/tr15/#Canon_Compat_Equivalence
//! [normalization forms]: http://www.unicode.org/reports/tr15/#Norm_Forms
//! [case folding]: https://www.w3.org/International/wiki/Case_folding
//! [(faq/casemap_charprop.html#2)]: http://unicode.org/faq/casemap_charprop.html#2
#![allow(dead_code)]

use std::fmt;
use std::ops;

use caseless::Caseless;
use unicode_normalization::{is_nfc, is_nfc_quick, IsNormalized, UnicodeNormalization};

/// A unicode string normalized with NFC.
///
/// Can be used transparently as a `&str`.
/// The inner `String` is private to ensure it remains normalized.
/// Uses default `String` order, which is probably not correct unicode order.
#[derive(Clone, Hash, Eq, PartialEq, Ord, PartialOrd)]
pub struct Nfc {
    inner: String,
}

impl Nfc {
    /// Creates a normalized caseless string.
    pub fn caseless(s: &str) -> Self {
        let string: String = if s.is_ascii() {
            s.to_ascii_lowercase()
        } else {
            s.chars().default_case_fold().collect()
        };
        Self::from(string)
    }

    /// Creates a normalized path string.
    /// Converts '\\' to '/'.
    pub fn path(s: &str) -> Self {
        let s = s.replace('\\', "/");
        Self::from(s)
    }

    /// Creates a normalized caseless path string.
    /// Converts '\\' to '/'.
    pub fn caseless_path(s: &str) -> Self {
        let s = s.replace('\\', "/");
        Self::caseless(&s)
    }

    /// Match `String::as_str()`.
    pub fn as_str(&self) -> &str {
        &self.inner
    }
}

impl AsRef<[u8]> for Nfc {
    fn as_ref(&self) -> &[u8] {
        self.inner.as_bytes()
    }
}

impl AsRef<str> for Nfc {
    fn as_ref(&self) -> &str {
        &self.inner
    }
}

impl From<&str> for Nfc {
    fn from(s: &str) -> Self {
        Self::from(s.to_owned())
    }
}

/// Converts to a normalized string.
/// Consumes the original string.
impl From<String> for Nfc {
    fn from(string: String) -> Self {
        match is_nfc_quick(string.chars()) {
            IsNormalized::Yes => Self { inner: string },
            IsNormalized::Maybe if is_nfc(&string) => Self { inner: string },
            _ => {
                let string = string.chars().nfc().collect();
                Self { inner: string }
            }
        }
    }
}

/// Unwraps the inner string.
impl Into<String> for Nfc {
    fn into(self) -> String {
        self.inner
    }
}

/// Unwraps the inner string into byte vector
impl Into<Vec<u8>> for Nfc {
    fn into(self) -> Vec<u8> {
        self.inner.into()
    }
}

/// Inherits all the methods of `str`.
impl ops::Deref for Nfc {
    type Target = str;

    fn deref(&self) -> &str {
        &self.inner
    }
}

/// Adds a string.
impl ops::Add<&str> for Nfc {
    type Output = Self;

    fn add(self, other: &str) -> Self {
        let string = self.inner.chars().chain(other.chars()).nfc().collect();
        Self { inner: string }
    }
}

/// Matches the inner string.
impl fmt::Display for Nfc {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let display: &dyn fmt::Display = &self.inner;
        display.fmt(f)
    }
}

/// Matches the inner string.
impl fmt::Debug for Nfc {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let debug: &dyn fmt::Debug = &self.inner;
        debug.fmt(f)
    }
}
