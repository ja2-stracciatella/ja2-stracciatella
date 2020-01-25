//! This module contains code to interact with the filesystem.
//!
//! Use this module instead of [`std::fs`].
//! It will make it easy to replace functions when needed.
//!
//! [`std::fs`]: https://doc.rust-lang.org/std/fs/index.html

use std::path::{Component, Path, PathBuf};

use dunce;

use crate::unicode::Nfc;

//------------
// re-exports
//------------

pub use std::fs::rename;

//--------------
// replacements
//--------------

/// Returns the canonical, absolute form of a path with all intermediate
/// components normalized and symbolic links resolved.
///
/// On windows, UNC paths are converted to normal paths when possible.
pub use dunce::canonicalize;

//-------
// other
//-------

/// Returns path joined with base.
/// The path separators are normalized and path components are resolved only when needed.
/// The returned path might or might not exist.
///
/// Variants are found by comparing caseless unicode in the NFC form.
/// Path components are replaced by their (correctly cased) variant when:
///  1) they are not part of base
///  2) caseless is true (respect the filesystem)
///  3) they don't exist (respect the filesystem)
///  4) a variant was found
///
/// Otherwise path components are copied without changes.
pub fn resolve_existing_components(path: &Path, base: Option<&Path>, caseless: bool) -> PathBuf {
    let joined_path;
    let mut copy = 0;
    if let Some(b) = base {
        joined_path = b.join(path);
        if joined_path.starts_with(&b) {
            // copy base components
            copy = b.components().count();
        }
    } else {
        joined_path = path.to_owned();
    };
    if !caseless {
        // respect the filesystem, copy all components
        copy = joined_path.components().count();
    }
    // normalize separators, copy and resolve the path components
    joined_path
        .components()
        .fold(PathBuf::new(), |mut current, component| {
            if copy > 0 {
                copy -= 1;
                current.push(&component);
                return current; // copied
            }
            current.push(&component);
            if current.exists() {
                return current; // respect the filesystem, copied
            }
            current.pop();
            if let Component::Normal(os_str) = component {
                // find caseless match (unicode in NFC form)
                let found = os_str.to_str().map(Nfc::caseless).and_then(|want| {
                    current.read_dir().ok().and_then(|entries| {
                        entries
                            .filter_map(|x| x.ok())
                            .map(|e| e.file_name())
                            .find(|f| f.to_str().map(|x| Nfc::caseless(x) == want) == Some(true))
                    })
                });
                if let Some(file_name) = found {
                    current.push(&file_name);
                    return current; // replaced with variant
                }
            }
            current.push(&component);
            current // give up, copied
        })
}
