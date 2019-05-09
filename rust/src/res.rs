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
//! | sha1              | 160 bits, SHA-1, output of sha1sum |
//! | blake2s           | 256 bits, BLAKE2s |
//! | blake2b           | 512 bits, BLAKE2b, default output of b2sum |
//!
//! Resource properties:
//!  * `hash_{algorithm}` (string) - hash of the specified algorithm
//!

use std::collections::VecDeque;
use std::convert::From;
use std::error::Error;
use std::fmt;
use std::io;
use std::path::{Path, PathBuf};

use serde::{Deserialize, Serialize};
use serde_json::{json, Map, Value};

use blake2::{Blake2b, Blake2s};
use digest::Digest;
use hex;
use md5::Md5;
use sha1::Sha1;

use crate::slf::{SlfEntryState, SlfHeader};

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
    with_archives: Vec<String>,

    /// Add file size to the resource properties.
    with_file_size: bool,

    /// Add hashes of the data to the resource properties.
    with_hashes: Vec<String>,

    /// Add paths to the pack (base, path).
    with_paths: VecDeque<(PathBuf, PathBuf)>,

    /// Resource being built.
    pack: ResourcePack,
}

impl ResourcePack {
    // Constructor.
    #[allow(dead_code)]
    fn new(name: &str) -> Self {
        let mut pack = ResourcePack::default();
        pack.name = name.to_owned();
        return pack;
    }
}

impl Resource {
    // Constructor.
    #[allow(dead_code)]
    fn new(path: &str) -> Self {
        let mut resource = Resource::default();
        resource.path = path.to_owned();
        return resource;
    }
}

impl ResourcePackBuilder {
    // Constructor.
    #[allow(dead_code)]
    pub fn new() -> Self {
        return Self::default();
    }

    /// Adds archive contents.
    #[allow(dead_code)]
    pub fn with_archive(&mut self, extension: &str) -> &mut Self {
        self.with_archives.push(extension.to_owned());
        return self;
    }

    /// Adds file sizes.
    #[allow(dead_code)]
    pub fn with_file_size(&mut self) -> &mut Self {
        self.with_file_size = true;
        return self;
    }

    /// Adds a hash algorithm.
    #[allow(dead_code)]
    pub fn with_hash(&mut self, algorithm: &str) -> &mut Self {
        self.with_hashes.push(algorithm.to_owned());
        return self;
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
        return self;
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
                "md5" | "sha1" | "blake2s" | "blake2b" => {}
                _ => return Err(format!("{:?} hashes are not supported", algorithm).into()),
            }
            let prop = "with_hash_".to_owned() + algorithm;
            self.pack.set_property(&prop, true);
        }
        while self.with_paths.len() > 0 {
            let (base, path) = self.with_paths.pop_front().unwrap();
            let metadata = path.metadata()?;
            if metadata.is_file() {
                self.add_file(&base, &path)?;
            } else if metadata.is_dir() {
                self.add_dir_contents(&base, &path)?;
            }
        }
        let pack = self.pack.to_owned();
        self.pack = ResourcePack::default();
        return Ok(pack);
    }

    /// Adds the contents of an OS directory.
    fn add_dir_contents(&mut self, base: &Path, dir: &Path) -> Result<(), ResourceError> {
        for path in FSIterator::new(dir) {
            self.add_file(&base, &path)?;
        }
        return Ok(());
    }

    /// Adds an OS file as a resource.
    fn add_file(&mut self, base: &Path, path: &Path) -> Result<(), ResourceError> {
        // must have a valid resource path
        let resource_path = resource_path(base, path)?;
        let mut resource = Resource::new(&resource_path);
        if self.with_file_size {
            resource.set_property("file_size", path.metadata()?.len());
        }
        let extension = lowercase_extension(path);
        let wants_archive = self.with_archives.binary_search(&extension).is_ok();
        let wants_hashes = self.with_hashes.len() > 0;
        if wants_archive || wants_hashes {
            let data = std::fs::read(path)?;
            if wants_archive {
                match extension.as_str() {
                    "slf" => self.add_slf_contents(&mut resource, &data)?,
                    _ => panic!(), // execute() must be fixed
                }
            }
            if wants_hashes {
                self.add_hashes(&mut resource, &data)?;
            }
        }
        self.pack.resources.push(resource);
        return Ok(());
    }

    // Adds the contents of a SLF archive.
    fn add_slf_contents(&mut self, slf: &mut Resource, data: &[u8]) -> Result<(), ResourceError> {
        slf.set_property("archive_slf", true);
        let mut num_resources = 0;
        let mut input = io::Cursor::new(&data);
        let header = SlfHeader::from_input(&mut input)?;
        for entry in header.entries_from_input(&mut input)? {
            if entry.state == SlfEntryState::Ok {
                let mut resource = Resource::default();
                let path = header.library_path.clone() + &entry.file_path;
                resource.path = path.replace("\\", "/");
                resource.set_property("archive_path", &slf.path);
                // record file size
                if self.with_file_size {
                    resource.set_property("file_size", entry.length);
                }
                // TODO include archive inside archive?
                let wants_hashes = self.with_hashes.len() > 0;
                if wants_hashes {
                    let from = entry.offset as usize;
                    let to = from + entry.length as usize;
                    self.add_hashes(&mut resource, &data[from..to])?;
                }
                self.pack.resources.push(resource);
                num_resources += 1;
            }
        }
        slf.set_property("archive_slf_num_resources", num_resources);
        return Ok(());
    }

    /// Adds hashes of the resource data.
    fn add_hashes(&mut self, resource: &mut Resource, data: &[u8]) -> Result<(), ResourceError> {
        for algorithm in &self.with_hashes {
            let hash = match algorithm.as_str() {
                "md5" => hex::encode(Md5::digest(&data)),
                "sha1" => hex::encode(Sha1::digest(&data)),
                "blake2s" => hex::encode(Blake2s::digest(&data)),
                "blake2b" => hex::encode(Blake2b::digest(&data)),
                _ => panic!(), // execute() must be fixed
            };
            let prop = "hash_".to_owned() + algorithm;
            resource.set_property(&prop, hash);
        }
        return Ok(());
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

impl Error for ResourceError {
    fn description(&self) -> &str {
        return match self {
            ResourceError::Text(desc) => desc,
            ResourceError::IoError(err) => err.description(),
        };
    }
}

impl fmt::Display for ResourceError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.description())
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
        return all_files;
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
fn lowercase_extension(path: &Path) -> String {
    if let Some(os_ext) = path.extension() {
        if let Some(ext) = os_ext.to_str() {
            // supported extensions are all ASCII
            return ext.to_ascii_lowercase();
        }
    }
    return "".to_owned();
}

/// Converts an OS path to a resource path.
fn resource_path(base: &Path, path: &Path) -> Result<String, ResourceError> {
    if let Ok(resource_path) = path.strip_prefix(base) {
        // there can be invalid utf8 in the prefix, but not in the resource path
        if let Some(utf8) = resource_path.to_str() {
            return Ok(utf8.replace("\\", "/"));
        }
        return Err(format!("{:?} contains invalid utf8", resource_path).into());
    }
    return Err(format!("{:?} is not a prefix of {:?}", base, path).into());
}

/// Trait the adds shortcuts for properties.
pub trait ResourcePropertiesExt {
    /// Gets a reference to the properties container.
    fn properties(&self) -> &Map<String, Value>;

    /// Gets a mutable reference to the properties container.
    fn properties_mut(&mut self) -> &mut Map<String, Value>;

    /// Removes a property and returns the old value.
    fn remove_property(&mut self, name: &str) -> Option<Value> {
        return self.properties_mut().remove(name);
    }

    /// Sets the value of a property and returns the old value.
    fn set_property<T: Serialize>(&mut self, name: &str, value: T) -> Option<Value> {
        return self.properties_mut().insert(name.to_owned(), json!(value));
    }

    /// Gets the value of a property.
    fn get_property(&self, name: &str) -> Option<&Value> {
        return self.properties().get(name);
    }

    /// Gets the value of a bool property.
    fn get_bool(&self, name: &str) -> Option<bool> {
        return self.get_property(name).and_then(|v| v.as_bool());
    }

    /// Gets the value of a string property.
    fn get_str(&self, name: &str) -> Option<&str> {
        return self.get_property(name).and_then(|v| v.as_str());
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
        return None;
    }

    /// Gets the signed integer value of a number property.
    fn get_i64(&self, name: &str) -> Option<i64> {
        return self.get_property(name).and_then(|v| v.as_i64());
    }

    /// Gets the unsigned integer value of a number property.
    fn get_u64(&self, name: &str) -> Option<u64> {
        return self.get_property(name).and_then(|v| v.as_u64());
    }

    /// Gets the floating-point value of a number property.
    fn get_f64(&self, name: &str) -> Option<f64> {
        return self.get_property(name).and_then(|v| v.as_f64());
    }
}

impl ResourcePropertiesExt for ResourcePack {
    fn properties(&self) -> &Map<String, Value> {
        return &self.properties;
    }

    fn properties_mut(&mut self) -> &mut Map<String, Value> {
        return &mut self.properties;
    }
}

impl ResourcePropertiesExt for Resource {
    fn properties(&self) -> &Map<String, Value> {
        return &self.properties;
    }

    fn properties_mut(&mut self) -> &mut Map<String, Value> {
        return &mut self.properties;
    }
}

#[cfg(test)]
mod tests {
    use super::{Resource, ResourcePropertiesExt};

    #[test]
    fn property_value_compatibility() {
        let mut resource = Resource::default();
        // boolean
        resource.set_property("p", true);
        assert!(resource.get_bool("p").is_some());
        assert!(resource.get_str("p").is_none());
        assert!(resource.get_i64("p").is_none());
        assert!(resource.get_u64("p").is_none());
        assert!(resource.get_f64("p").is_none());
        assert_eq!(resource.get_bool("p").unwrap(), true);
        // string
        resource.set_property("p", "foo");
        assert!(resource.get_bool("p").is_none());
        assert!(resource.get_str("p").is_some());
        assert!(resource.get_i64("p").is_none());
        assert!(resource.get_u64("p").is_none());
        assert!(resource.get_f64("p").is_none());
        assert_eq!(resource.get_str("p").unwrap(), "foo");
        // universal number
        resource.set_property("p", 0);
        assert!(resource.get_bool("p").is_none());
        assert!(resource.get_str("p").is_none());
        assert!(resource.get_i64("p").is_some());
        assert!(resource.get_u64("p").is_some());
        assert!(resource.get_f64("p").is_some());
        assert_eq!(resource.get_i64("p").unwrap(), 0);
        // floating point number
        resource.set_property("p", 0.5);
        assert!(resource.get_bool("p").is_none());
        assert!(resource.get_str("p").is_none());
        assert!(resource.get_i64("p").is_none());
        assert!(resource.get_u64("p").is_none());
        assert!(resource.get_f64("p").is_some());
        assert_eq!(resource.get_f64("p").unwrap(), 0.5);
        // negative number
        resource.set_property("p", -1);
        assert!(resource.get_bool("p").is_none());
        assert!(resource.get_str("p").is_none());
        assert!(resource.get_i64("p").is_some());
        assert!(resource.get_u64("p").is_none());
        assert!(resource.get_f64("p").is_some());
        assert_eq!(resource.get_i64("p").unwrap(), -1);
        // big number
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

        use blake2::{Blake2b, Blake2s};
        use digest::Digest;
        use hex;
        use md5::Md5;
        use sha1::Sha1;

        fn data_for_hasher() -> Vec<u8> {
            return "A quick brown fox jumps over the lazy dog"
                .repeat(1_000_000)
                .as_bytes()
                .to_vec();
        }

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

        #[test]
        #[ignore]
        fn hasher_sha1() {
            let data = data_for_hasher();
            let start = SystemTime::now();
            let hash = Sha1::digest(&data);
            let elapsed = start.elapsed().unwrap();
            print_hasher_result("hasher_sha1", elapsed, data.len(), &hash);
        }

        #[test]
        #[ignore]
        fn hasher_blake2s() {
            let data = data_for_hasher();
            let start = SystemTime::now();
            let hash = Blake2s::digest(&data);
            let elapsed = start.elapsed().unwrap();
            print_hasher_result("hasher_blake2s", elapsed, data.len(), &hash);
        }

        #[test]
        #[ignore]
        fn hasher_blake2b() {
            let data = data_for_hasher();
            let start = SystemTime::now();
            let hash = Blake2b::digest(&data);
            let elapsed = start.elapsed().unwrap();
            print_hasher_result("hasher_blake2b", elapsed, data.len(), &hash);
        }
    }
}
