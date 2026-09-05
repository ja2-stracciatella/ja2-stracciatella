//! This module implements a writable VFS layer backed by a temporary directory.

use std::collections::BTreeSet;
use std::fmt;
use std::io;
use std::path::Path;
use std::sync::Arc;

use crate::fs::{TempBuilder, TempDir};
use crate::unicode::Nfc;
use crate::vfs::dir::DirFs;
use crate::vfs::{VfsFile, VfsLayer, VfsMetadata, VfsOpenOptions};

/// A writable virtual filesystem backed by a temporary directory that it owns.
///
/// The directory and everything in it is deleted when the layer is dropped, so the scratch files
/// the engine writes while saving a game do not outlive the layer they were written to.
#[derive(Debug)]
pub struct TmpFs {
    /// The temporary directory. Deleted on drop, which is why it is kept around.
    temp_dir: TempDir,
    /// Does the actual work, the temporary directory is a plain directory after all.
    dir: Arc<DirFs>,
}

/// Creates a temporary directory that is deleted together with its contents when it is dropped.
///
/// On android the operating system temporary directory is not usable, so the cache directory of
/// the application is used instead.
fn create_temp_dir() -> io::Result<TempDir> {
    let mut builder = TempBuilder::new();
    builder.prefix("ja2-stracciatella-");
    builder.rand_bytes(8);

    #[cfg(target_os = "android")]
    {
        let cache_dir = crate::android::get_android_cache_dir()
            .map_err(|err| io::Error::other(err.to_string()))?;
        builder.tempdir_in(cache_dir)
    }
    #[cfg(not(target_os = "android"))]
    {
        builder.tempdir()
    }
}

impl TmpFs {
    /// Creates a new temporary directory and a writable layer for it.
    pub fn new() -> io::Result<Arc<TmpFs>> {
        let temp_dir = create_temp_dir()?;
        let dir = DirFs::new_writable(temp_dir.path())?;
        Ok(Arc::new(TmpFs { temp_dir, dir }))
    }

    /// The path of the temporary directory.
    pub fn path(&self) -> &Path {
        self.temp_dir.path()
    }
}

impl VfsLayer for TmpFs {
    fn open(&self, file_path: &Nfc) -> io::Result<Box<dyn VfsFile>> {
        self.dir.open(file_path)
    }

    fn exists(&self, file_path: &Nfc) -> io::Result<bool> {
        self.dir.exists(file_path)
    }

    fn read_dir(&self, file_path: &Nfc) -> io::Result<BTreeSet<Nfc>> {
        self.dir.read_dir(file_path)
    }

    fn is_writable(&self) -> bool {
        true
    }

    fn metadata(&self, file_path: &Nfc) -> io::Result<VfsMetadata> {
        self.dir.metadata(file_path)
    }

    fn resolve_existing_components(&self, file_path: &Nfc) -> Nfc {
        self.dir.resolve_existing_components(file_path)
    }

    fn open_with_options(
        &self,
        file_path: &Nfc,
        options: VfsOpenOptions,
    ) -> io::Result<Box<dyn VfsFile>> {
        self.dir.open_with_options(file_path, options)
    }

    fn remove_file(&self, file_path: &Nfc) -> io::Result<()> {
        self.dir.remove_file(file_path)
    }

    fn create_dir(&self, file_path: &Nfc) -> io::Result<()> {
        self.dir.create_dir(file_path)
    }

    fn rename(&self, from: &Nfc, to: &Nfc) -> io::Result<()> {
        self.dir.rename(from, to)
    }

    fn free_space(&self) -> io::Result<u64> {
        self.dir.free_space()
    }
}

impl fmt::Display for TmpFs {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "TmpFs {{ {:?} }}", self.path())
    }
}
