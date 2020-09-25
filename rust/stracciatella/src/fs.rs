//! This module contains code to interact with the filesystem.
//!
//! Use this module instead of [`std::fs`].
//! It will make it easy to replace functions when needed.
//!
//! [`std::fs`]: https://doc.rust-lang.org/std/fs/index.html

use std::io;
use std::path::{Component, Path, PathBuf};

use dunce;

use crate::unicode::Nfc;

//------------
// re-exports
//------------

pub use std::fs::create_dir;
pub use std::fs::create_dir_all;
pub use std::fs::metadata;
pub use std::fs::read;
pub use std::fs::read_dir;
pub use std::fs::rename;
pub use std::fs::set_permissions;
pub use std::fs::write;
pub use std::fs::File;
pub use std::fs::OpenOptions;

pub use tempfile::TempDir;

//--------------
// replacements
//--------------

/// A reliable implementation of remove_dir_all for Windows.
/// For other systems it re-exports std::fs::remove_dir_all.
pub use remove_dir_all::remove_dir_all;

/// Returns the canonical, absolute form of a path with all intermediate
/// components normalized and symbolic links resolved.
///
/// On windows, UNC paths are converted to normal paths when possible.
pub use dunce::canonicalize;

/// An implementation of `std::fs::remove_file` that handles the readonly permission on windows.
pub fn remove_file<P: AsRef<Path>>(path: P) -> Result<(), io::Error> {
    #[cfg(windows)]
    {
        // On windows a readonly file cannot be deleted.
        // This simplistic solution removes the readonly permission.
        // For a proper solution look at the unlink_nt syscall in cygwin: https://github.com/cygwin/cygwin/blob/master/winsup/cygwin/syscalls.cc
        let mut permissions = metadata(&path)?.permissions();
        if permissions.readonly() {
            permissions.set_readonly(false);
            set_permissions(&path, permissions)?;
        }
    }
    std::fs::remove_file(path)
}

//-------
// other
//-------

/// Returns base (optional) joined with path.
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

/// Gets the paths of the directory entries.
pub fn read_dir_paths(dir: &Path, ignore_entry_errors: bool) -> io::Result<Vec<PathBuf>> {
    let mut vec = Vec::new();
    for entry_result in read_dir(&dir)? {
        match entry_result {
            Ok(entry) => {
                vec.push(entry.path());
            }
            Err(err) => {
                if !ignore_entry_errors {
                    return Err(err);
                }
            }
        }
    }
    Ok(vec)
}

/// Gets the amount of bytes available as space in the target path.
#[allow(unreachable_code)]
pub fn free_space(path: &Path) -> io::Result<u64> {
    // I did not find a crate with this functionality.
    // This is a "best effort" implementation.
    // It might not compile or be unimplemented on some targets.
    #[cfg(windows)]
    {
        // use GetDiskFreeSpaceExW in the windows family (msvc/msys2)
        use std::os::windows::ffi::OsStrExt;

        use winapi::um::fileapi::GetDiskFreeSpaceExW;
        use winapi::um::winnt::ULARGE_INTEGER;

        let wpath: Vec<u16> = path
            .as_os_str()
            .encode_wide()
            .chain(std::iter::once(0))
            .collect();
        let mut free_bytes: ULARGE_INTEGER = unsafe { std::mem::zeroed() };
        let result = unsafe {
            GetDiskFreeSpaceExW(
                wpath.as_ptr(),
                std::ptr::null_mut(),
                std::ptr::null_mut(),
                &mut free_bytes,
            )
        };
        return match result {
            0 => Err(io::Error::last_os_error()),
            _ => Ok(*unsafe { free_bytes.QuadPart() }),
        };
    }
    #[cfg(all(unix, not(target_os = "android")))]
    #[allow(clippy::cast_lossless)]
    {
        // use statvfs in unix family
        // TODO determine which of statvfs64/statfs64/statvfs/statfs are available
        use std::os::unix::ffi::OsStrExt;

        let mut bytes = path.as_os_str().as_bytes().to_vec();
        bytes.push(0);
        let mut data: libc::statvfs = unsafe { std::mem::zeroed() };
        let result = unsafe { libc::statvfs(bytes.as_ptr() as *const libc::c_char, &mut data) };
        return match result {
            0 => Ok(data.f_bfree as u64 * data.f_bsize as u64),
            _ => Err(io::Error::last_os_error()),
        };
    }
    #[cfg(target_os = "android")]
    #[allow(clippy::cast_lossless)]
    {
        return Ok(1024 * 1024 * 1024);
    }
    Err(io::Error::new(io::ErrorKind::Other, "not implemented"))
}
