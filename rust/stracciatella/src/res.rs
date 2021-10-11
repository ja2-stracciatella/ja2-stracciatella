//! This file contains code to generate resource packs.
//!
//! A resource pack identifies the resources in a particular game version/mod.
//!
//! Required data lives directly in the structs, the rest is optional and goes in the properties.
//!
//! The properties are arbitrary on purpose. It is the only place where whoever manages the file can
//! place additional information like an author, an url, a revision, a license, any valid json really.
//!
//! Some properties have specific meanings.
//!
//!
//! # `with_archive_{format}` (bool, default = false)
//!
//! An archive is a file that holds a collection of files inside it.
//!
//! When this pack property is set to `true`, the resource pack includes the files inside that type of archive as resources.
//!
//! Supported formats:
//!  * `slf` - SLF files from the Jagged Alliance 2 series or mods
//!
//! Resource properties:
//!  * `archive_{format}` (bool) - `true` on resources that represent archives of the specified format
//!  * `archive_{format}_num_resources` (integer) - number of resources that were included from inside the archive
//!  * `archive_path` (string) - path of the archive resource that contains this file
//!
//! NOTE archives inside others archives are not supported, they are treated as regular files
//!
//!
//! # `with_file_size` (bool, default = false)
//!
//! Files have data.
//!
//! When this pack property is set to `true`, the resource properties include the size of the file data.
//!
//! Resource properties:
//!  * `file_size` (integer) - size of the file data.
//!
//!
//! # `with_hash_{algorithm}` (bool, default = false)
//!
//! File data can be hashed.
//! A hasher digests the data into a small fixed size and the same input produces the same output.
//!
//! When this pack property is true, the resource properties include the hash of the data in the specified algorithm.
//!
//! | Algorithms        | Notes |
//! |-------------------|-------|
//! | md5               | 128 bits, MD5, output of md5sum |
//!
//! Resource properties:
//!  * `hash_{algorithm}` (string) - hash of the specified algorithm
//!

use std::collections::{HashSet, VecDeque};
use std::convert::From;
use std::fmt;
use std::io;
use std::path::{Path, PathBuf};

use digest::Digest;
use md5::Md5;
use rayon::prelude::*;
use serde::{Deserialize, Serialize};
use serde_json::{json, Map, Value};

use crate::file_formats::slf::{SlfEntryState, SlfHeader};
use crate::unicode::Nfc;

/// A pack of game resources.
#[derive(Clone, Debug, Default, Deserialize, Serialize)]
pub struct ResourcePack {
    /// A friendly name of the resource pack for display purposes.
    pub name: String,

    /// The properties of the resource pack.
    pub properties: Map<String, Value>,

    /// The resources in this pack.
    pub resources: Vec<Resource>,
}

/// A resource in the pack.
///
/// A resource always maps to raw data, which can be a file or data inside an archive.
#[derive(Clone, Debug, Default, Deserialize, Serialize)]
pub struct Resource {
    /// The identity of the resource as a relative path.
    pub path: String,

    /// The properties of the resource.
    pub properties: Map<String, Value>,
}

#[derive(Clone, Debug, Default)]
pub struct ResourcePackBuilder {
    /// Include archive contents as resources.
    with_archives: Vec<Nfc>,

    /// Add file size to the resource properties.
    with_file_size: bool,

    /// Add hashes of the data to the resource properties.
    with_hashes: Vec<Nfc>,

    /// Add paths to the pack (base, path).
    with_paths: VecDeque<(PathBuf, PathBuf)>,

    /// Resource being built.
    pack: ResourcePack,
}

impl ResourcePack {
    /// Constructor.
    #[allow(dead_code)]
    fn new(name: &str) -> Self {
        let mut pack = ResourcePack::default();
        pack.name = name.to_owned();
        pack
    }

    /// Get properties that are enabled (true)
    #[allow(dead_code)]
    fn get_enabled_properties(&self) -> impl Iterator<Item = &String> {
        self.properties
            .iter()
            .filter(|(_, v)| v.as_bool() == Some(true))
            .map(|(k, _)| k)
    }

    /// Returns wether the resource pack has been built with file sizes
    #[allow(dead_code)]
    pub fn has_file_size(&self) -> bool {
        self.get_enabled_properties()
            .any(|k| k.as_str() == "with_file_size")
    }

    /// Returns the hashes that are used in the resource pack
    #[allow(dead_code)]
    pub fn get_hashes(&self) -> HashSet<String> {
        self.get_enabled_properties()
            .filter(|k| k.starts_with("with_hash_"))
            .map(|k| k["with_hash_".len()..].to_owned())
            .collect()
    }

    /// Returns which archives were inspected when building the resource pack
    #[allow(dead_code)]
    pub fn get_archives(&self) -> HashSet<String> {
        self.get_enabled_properties()
            .filter(|k| k.starts_with("with_archive_"))
            .map(|k| k["with_archive_".len()..].to_owned())
            .collect()
    }
}

impl Resource {
    // Constructor.
    #[allow(dead_code)]
    fn new(path: &str) -> Self {
        let mut resource = Resource::default();
        resource.path = path.to_owned();
        resource
    }
}

impl ResourcePackBuilder {
    // Constructor.
    #[allow(dead_code)]
    pub fn new() -> Self {
        Self::default()
    }

    /// Adds archive contents.
    #[allow(dead_code)]
    pub fn with_archive(&mut self, extension: &str) -> &mut Self {
        self.with_archives.push(extension.into());
        self
    }

    /// Adds file sizes.
    #[allow(dead_code)]
    pub fn with_file_size(&mut self) -> &mut Self {
        self.with_file_size = true;
        self
    }

    /// Adds a hash algorithm.
    #[allow(dead_code)]
    pub fn with_hash(&mut self, algorithm: &str) -> &mut Self {
        self.with_hashes.push(algorithm.into());
        self
    }

    /// Adds a directory or an archive.
    #[allow(dead_code)]
    pub fn with_path(&mut self, base: &Path, path: &Path) -> &mut Self {
        for (b, p) in &self.with_paths {
            if b == base && p == path {
                return self; // avoid duplicates
            }
        }
        let tuple = (base.to_owned(), path.to_owned());
        self.with_paths.push_back(tuple);
        self
    }

    /// Returns a resource pack or an error.
    #[allow(dead_code)]
    pub fn execute(&mut self, name: &str) -> Result<ResourcePack, ResourceError> {
        self.pack = ResourcePack::new(name);
        self.with_archives.sort();
        self.with_archives.dedup();
        for extension in &self.with_archives {
            match extension.as_str() {
                "slf" => {}
                _ => {
                    return Err(format!("{:?} archives are not supported", extension).into());
                }
            }
            let prop = "with_archive_".to_owned() + extension;
            self.pack.set_property(&prop, true);
        }
        if self.with_file_size {
            self.pack.set_property("with_file_size", true);
        }
        self.with_hashes.sort();
        self.with_hashes.dedup();
        for algorithm in &self.with_hashes {
            match algorithm.as_str() {
                "md5" => {}
                _ => return Err(format!("{:?} hashes are not supported", algorithm).into()),
            }
            let prop = "with_hash_".to_owned() + algorithm;
            self.pack.set_property(&prop, true);
        }

        let resources: Result<Vec<Vec<Resource>>, ResourceError> = self
            .with_paths
            .par_iter()
            .map(|paths| {
                let (base, path) = paths;
                let metadata = path.metadata()?;
                if metadata.is_file() {
                    self.get_resources_for_file(&base, &path)
                } else if metadata.is_dir() {
                    self.get_resources_for_dir(&base, &path)
                } else {
                    Ok(vec![])
                }
            })
            .collect();
        let mut resources: Vec<_> = resources?.into_iter().flatten().collect();
        self.pack.resources.append(&mut resources);

        let pack = self.pack.to_owned();
        self.pack = ResourcePack::default();
        Ok(pack)
    }

    /// Adds the contents of an OS directory.
    fn get_resources_for_dir(
        &self,
        base: &Path,
        dir: &Path,
    ) -> Result<Vec<Resource>, ResourceError> {
        let files: Vec<_> = FSIterator::new(dir).collect();
        let resources: Result<Vec<Vec<Resource>>, _> = files
            .par_iter()
            .map(|file| self.get_resources_for_file(base, &file))
            .collect();
        Ok(resources?.into_iter().flatten().collect())
    }

    /// Adds an OS file as a resource.
    fn get_resources_for_file(
        &self,
        base: &Path,
        path: &Path,
    ) -> Result<Vec<Resource>, ResourceError> {
        // must have a valid resource path
        let mut resources = vec![];
        let resource_path = resource_path(base, path)?;
        if resource_path.to_ascii_lowercase().starts_with("temp/") {
            // the game can create/modify/remove files in the temp folder, ignore it
            return Ok(resources);
        }
        let mut resource = Resource::new(&resource_path);
        if self.with_file_size {
            resource.set_property("file_size", path.metadata()?.len());
        }
        let extension = lowercase_extension(path);
        let wants_archive = self.with_archives.binary_search(&extension).is_ok();
        let wants_hashes = !self.with_hashes.is_empty();
        if wants_archive || wants_hashes {
            let data = std::fs::read(path)?;
            if wants_archive {
                match extension.as_str() {
                    "slf" => {
                        let mut slf_resources = self.get_resources_for_slf(&mut resource, &data)?;
                        resources.append(&mut slf_resources);
                    }
                    _ => panic!(), // execute() must be fixed
                }
            }
            if wants_hashes {
                self.add_hashes(&mut resource, &data)?;
            }
        }
        resources.push(resource);
        Ok(resources)
    }

    // Adds the contents of a SLF archive.
    fn get_resources_for_slf(
        &self,
        slf: &mut Resource,
        data: &[u8],
    ) -> Result<Vec<Resource>, ResourceError> {
        slf.set_property("archive_slf", true);
        let mut input = io::Cursor::new(&data);
        let header = SlfHeader::from_input(&mut input)?;
        let entries = header.entries_from_input(&mut input)?;
        let resources: Result<Vec<Resource>, ResourceError> = entries
            .par_iter()
            .filter(|entry| entry.state == SlfEntryState::Ok)
            .map(|entry| {
                let mut resource = Resource::default();
                let path = header.library_path.clone() + &entry.file_path;
                resource.path = path.replace("\\", "/");
                resource.set_property("archive_path", &slf.path);
                // record file size
                if self.with_file_size {
                    resource.set_property("file_size", entry.length);
                }
                let wants_hashes = !self.with_hashes.is_empty();
                if wants_hashes {
                    let from = entry.offset as usize;
                    let to = from + entry.length as usize;
                    self.add_hashes(&mut resource, &data[from..to])?;
                }
                Ok(resource)
            })
            .collect();
        let resources = resources?;
        slf.set_property("archive_slf_num_resources", resources.len());
        Ok(resources)
    }

    /// Adds hashes of the resource data.
    fn add_hashes(&self, resource: &mut Resource, data: &[u8]) -> Result<(), ResourceError> {
        for algorithm in &self.with_hashes {
            let hash = match algorithm.as_str() {
                "md5" => hex::encode(Md5::digest(&data)),
                _ => panic!(), // execute() must be fixed
            };
            let prop = "hash_".to_owned() + algorithm;
            resource.set_property(&prop, hash);
        }
        Ok(())
    }
}

/// Error that originates from this module.
#[derive(Debug)]
pub enum ResourceError {
    /// Simple text error.
    Text(String),
    /// Original error was an IO error.
    IoError(io::Error),
}

impl fmt::Display for ResourceError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "ResourceError(")?;
        match self {
            ResourceError::Text(desc) => write!(f, "{}", desc),
            ResourceError::IoError(err) => err.fmt(f),
        }?;
        write!(f, ")")?;
        Ok(())
    }
}

impl From<String> for ResourceError {
    fn from(desc: String) -> ResourceError {
        ResourceError::Text(desc)
    }
}

impl From<io::Error> for ResourceError {
    fn from(err: io::Error) -> ResourceError {
        ResourceError::IoError(err)
    }
}

/// Iterator that returns the paths of all the filesystem files in breadth-first order, files before dirs.
#[derive(Debug, Default)]
struct FSIterator {
    files: VecDeque<PathBuf>,
    dirs: VecDeque<PathBuf>,
}

impl FSIterator {
    fn new(path: &Path) -> Self {
        let mut all_files = Self::default();
        all_files.with(path.to_owned());
        all_files
    }

    /// Adds a path to the iterator.
    fn with(&mut self, path: PathBuf) {
        if path.is_file() {
            self.files.push_back(path);
        } else if path.is_dir() {
            self.dirs.push_back(path);
        }
    }
}

impl Iterator for FSIterator {
    type Item = PathBuf;

    /// Get the next file path while ignoring errors.
    fn next(&mut self) -> Option<Self::Item> {
        loop {
            if let Some(file) = self.files.pop_front() {
                return Some(file);
            } else if let Some(dir) = self.dirs.pop_front() {
                // TODO detect and skip circles
                if let Ok(iter) = dir.read_dir() {
                    for entry in iter {
                        if let Ok(path) = entry.map(|entry| entry.path()) {
                            self.with(path);
                        }
                    }
                }
            } else {
                // we're done
                return None;
            }
        }
    }
}

/// Returns the lowercase extension.
fn lowercase_extension(path: &Path) -> Nfc {
    path.extension()
        .and_then(|x| x.to_str())
        .map(|x| x.to_lowercase())
        .unwrap_or_default()
        .into()
}

/// Converts an OS path to a resource path.
fn resource_path(base: &Path, path: &Path) -> Result<Nfc, ResourceError> {
    if let Ok(resource_path) = path.strip_prefix(base) {
        return match resource_path.to_str() {
            Some(utf8) => Ok(Nfc::path(utf8)),
            None => Err(format!("{:?} contains invalid utf8", resource_path).into()),
        };
    }
    Err(format!("{:?} is not a prefix of {:?}", base, path).into())
}

/// Trait the adds shortcuts for properties.
pub trait ResourcePropertiesExt {
    /// Gets a reference to the properties container.
    fn properties(&self) -> &Map<String, Value>;

    /// Gets a mutable reference to the properties container.
    fn properties_mut(&mut self) -> &mut Map<String, Value>;

    /// Removes a property and returns the old value.
    fn remove_property(&mut self, name: &str) -> Option<Value> {
        self.properties_mut().remove(name)
    }

    /// Sets the value of a property and returns the old value.
    fn set_property<T: Serialize>(&mut self, name: &str, value: T) -> Option<Value> {
        self.properties_mut().insert(name.to_owned(), json!(value))
    }

    /// Gets the value of a property.
    fn get_property(&self, name: &str) -> Option<&Value> {
        self.properties().get(name)
    }

    /// Gets the value of a bool property.
    fn get_bool(&self, name: &str) -> Option<bool> {
        self.get_property(name).and_then(|v| v.as_bool())
    }

    /// Gets the value of a string property.
    fn get_str(&self, name: &str) -> Option<&str> {
        self.get_property(name).and_then(|v| v.as_str())
    }

    /// Gets the value of an array of strings property.
    fn get_vec_of_str(&self, name: &str) -> Option<Vec<&str>> {
        if let Some(array) = self.get_property(name).and_then(|v| v.as_array()) {
            let mut strings = Vec::new();
            for value in array {
                if !value.is_string() {
                    return None;
                }
                strings.push(value.as_str().unwrap());
            }
            return Some(strings);
        }
        None
    }

    /// Gets the signed integer value of a number property.
    fn get_i64(&self, name: &str) -> Option<i64> {
        self.get_property(name).and_then(|v| v.as_i64())
    }

    /// Gets the unsigned integer value of a number property.
    fn get_u64(&self, name: &str) -> Option<u64> {
        self.get_property(name).and_then(|v| v.as_u64())
    }

    /// Gets the floating-point value of a number property.
    fn get_f64(&self, name: &str) -> Option<f64> {
        self.get_property(name).and_then(|v| v.as_f64())
    }
}

impl ResourcePropertiesExt for ResourcePack {
    fn properties(&self) -> &Map<String, Value> {
        &self.properties
    }

    fn properties_mut(&mut self) -> &mut Map<String, Value> {
        &mut self.properties
    }
}

impl ResourcePropertiesExt for Resource {
    fn properties(&self) -> &Map<String, Value> {
        &self.properties
    }

    fn properties_mut(&mut self) -> &mut Map<String, Value> {
        &mut self.properties
    }
}

#[cfg(test)]
mod tests {
    use crate::res::{Resource, ResourcePropertiesExt};

    #[test]
    fn property_value_compatibility_boolean() {
        let mut resource = Resource::default();
        resource.set_property("p", true);
        assert!(resource.get_bool("p").is_some());
        assert!(resource.get_str("p").is_none());
        assert!(resource.get_i64("p").is_none());
        assert!(resource.get_u64("p").is_none());
        assert!(resource.get_f64("p").is_none());
        assert_eq!(resource.get_bool("p").unwrap(), true);
    }

    #[test]
    fn property_value_compatibility_string() {
        let mut resource = Resource::default();
        resource.set_property("p", "foo");
        assert!(resource.get_bool("p").is_none());
        assert!(resource.get_str("p").is_some());
        assert!(resource.get_i64("p").is_none());
        assert!(resource.get_u64("p").is_none());
        assert!(resource.get_f64("p").is_none());
        assert_eq!(resource.get_str("p").unwrap(), "foo");
    }

    #[test]
    fn property_value_compatibility_universal_number() {
        let mut resource = Resource::default();
        resource.set_property("p", 0);
        assert!(resource.get_bool("p").is_none());
        assert!(resource.get_str("p").is_none());
        assert!(resource.get_i64("p").is_some());
        assert!(resource.get_u64("p").is_some());
        assert!(resource.get_f64("p").is_some());
        assert_eq!(resource.get_i64("p").unwrap(), 0);
    }

    #[test]
    #[allow(clippy::float_cmp)]
    fn property_value_compatibility_floating_point_number() {
        let mut resource = Resource::default();
        resource.set_property("p", 0.5);
        assert!(resource.get_bool("p").is_none());
        assert!(resource.get_str("p").is_none());
        assert!(resource.get_i64("p").is_none());
        assert!(resource.get_u64("p").is_none());
        assert!(resource.get_f64("p").is_some());
        assert_eq!(resource.get_f64("p").unwrap(), 0.5);
    }

    #[test]
    fn property_value_compatibility_negative_number() {
        let mut resource = Resource::default();
        resource.set_property("p", -1);
        assert!(resource.get_bool("p").is_none());
        assert!(resource.get_str("p").is_none());
        assert!(resource.get_i64("p").is_some());
        assert!(resource.get_u64("p").is_none());
        assert!(resource.get_f64("p").is_some());
        assert_eq!(resource.get_i64("p").unwrap(), -1);
    }

    #[test]
    fn property_value_compatibility_big_number() {
        let mut resource = Resource::default();
        resource.set_property("p", u64::max_value());
        assert!(resource.get_bool("p").is_none());
        assert!(resource.get_str("p").is_none());
        assert!(resource.get_i64("p").is_none());
        assert!(resource.get_u64("p").is_some());
        assert!(resource.get_f64("p").is_some());
        assert_eq!(resource.get_u64("p").unwrap(), u64::max_value());
    }

    // Since #[bench] isn't stable, these simple timed tests are ignored and print the times by themselves.
    // Execute them with `cargo test -- --nocapture --ignored`
    mod timed {
        use std::time::{Duration, SystemTime};

        use digest::Digest;
        use md5::Md5;

        fn data_for_hasher() -> Vec<u8> {
            "A quick brown fox jumps over the lazy dog"
                .repeat(1_000_000)
                .as_bytes()
                .to_vec()
        }

        #[allow(clippy::cast_lossless)]
        fn print_hasher_result(name: &str, time: Duration, size: usize, hash: &[u8]) {
            let secs = time.as_secs() as f64 + time.subsec_nanos() as f64 / 1_000_000_000f64;
            let mib = size as f64 / 1_000_000f64;
            let speed = mib / secs;
            let hash = hex::encode(hash);
            println!(
                "{}: {} bytes in {:?} {:.3}MiB/s {:?}",
                name, size, time, speed, hash
            );
        }

        #[test]
        #[ignore]
        fn hasher_md5() {
            let data = data_for_hasher();
            let start = SystemTime::now();
            let hash = Md5::digest(&data);
            let elapsed = start.elapsed().unwrap();
            print_hasher_result("hasher_md5", elapsed, data.len(), &hash);
        }
    }
}
