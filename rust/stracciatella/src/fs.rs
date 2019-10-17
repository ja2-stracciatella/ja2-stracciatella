//! This module contains code to interact with the filesystem.

use crate::unicode::Nfc;
use dunce;
use std::path::{Component, Path, PathBuf};

/// Returns the canonical, absolute form of a path with all intermediate
/// components normalized and symbolic links resolved.
///
/// On windows, UNC paths are converted to normal paths when possible.
pub use dunce::canonicalize;

/// Resolves all components of a path to their existing variant using unicode normalization.
/// Can be run in caseless mode.
/// The return value of the function has all components of the path that exist
/// in the file system replaced by their (correctly cased) variant.
pub fn resolve_existing_components(path: &Path, base: Option<&Path>, caseless: bool) -> PathBuf {
    let to_nfc = |x: &str| {
        if caseless {
            Nfc::caseless(x)
        } else {
            Nfc::from(x)
        }
    };
    let path = if let Some(b) = base {
        b.join(path)
    } else {
        path.to_owned()
    };
    path.components()
        .fold(PathBuf::new(), |mut current, component| {
            let next = match component {
                Component::CurDir => Component::CurDir.as_os_str().to_owned(),
                Component::ParentDir => Component::ParentDir.as_os_str().to_owned(),
                Component::Normal(os_str) => {
                    let existing = os_str.to_str().map(to_nfc).and_then(|want_caseless| {
                        current.read_dir().ok().and_then(|entries| {
                            entries
                                .filter_map(|x| x.ok())
                                .map(|entry| entry.file_name())
                                .find(|file_name| {
                                    file_name.to_str().map(to_nfc).map(|f| f == want_caseless)
                                        == Some(true)
                                })
                        })
                    });
                    existing.unwrap_or_else(|| os_str.to_owned())
                }
                Component::Prefix(e) => Component::Prefix(e).as_os_str().to_owned(),
                Component::RootDir => Component::RootDir.as_os_str().to_owned(),
            };
            current.push(next.as_os_str());
            current
        })
}
