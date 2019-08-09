//! This module contains code to guess Vanillaversion with resource packs.

use log::{error, info};
use std::convert::From;
use std::error::Error;
use std::ffi::OsString;
use std::fmt;
use std::fs::File;
use std::io;
use std::path::{Path, PathBuf};
use std::str::FromStr;

use serde_json;

use crate::config::VanillaVersion;
use crate::res::{
    Resource, ResourceError, ResourcePack, ResourcePackBuilder, ResourcePropertiesExt,
};
use crate::unicode::Nfc;

/// Guess the vanilla version of the resources in the game dir.
pub fn guess_vanilla_version(gamedir: &str) -> Guess {
    let path = Path::new(gamedir);
    let mut logged = Guess::default();
    if let Err(err) = logged.guess_vanilla_version(&path) {
        error!("Error: {}", err.desc);
    }
    logged
}

#[derive(Default)]
pub struct Guess {
    pub vanilla_version: Option<VanillaVersion>,
}

type GuessResult<T> = Result<T, GuessError>;

impl Guess {
    fn guess_vanilla_version(&mut self, gamedir: &Path) -> GuessResult<()> {
        let datadir = self.get_datadir(gamedir)?;
        let mut best_version = None;
        let mut best_extras = Vec::new();
        for path in self.get_pack_paths()? {
            match self.compare_pack(&datadir, &path) {
                Ok((version, extras)) => {
                    if extras.len() == 0 {
                        info!("Found perfect match with vanilla_version {:?}", version);
                        self.vanilla_version = Some(version);
                        return Ok(());
                    }
                    info!("Found match with vanilla_version {:?}", version);
                    for resource in &extras {
                        if let Some(archive_path) = resource.get_str("archive_path") {
                            info!(
                                "Extra resource: {:?} inside of {:?}",
                                &resource.path, archive_path
                            );
                        } else {
                            info!("Extra resource: {:?}", &resource.path);
                        }
                    }
                    if best_version.is_none() || best_extras.len() > extras.len() {
                        info!("New best match");
                        best_version = Some(version);
                        best_extras = extras;
                    }
                }
                Err(err) => {
                    info!("Error: {}", err.desc);
                }
            }
        }
        info!("No more resource packs");
        if best_version.is_some() {
            // TODO inspect the extra resources? how to handle mod files inside datadir?
            let version = best_version.unwrap();
            info!("Using best match with vanilla_version {:?}", version);
            self.vanilla_version = best_version;
            return Ok(());
        }
        // Last ditch effort with resource paths.
        info!("Getting resources with_archive_slf");
        let resources = ResourcePackBuilder::new()
            .with_path(&datadir, &datadir)
            .with_archive("slf")
            .execute("paths")?
            .resources;
        info!("Inspecting resource paths");
        let mut num_dutch = 0;
        let mut num_german = 0;
        let mut num_italian = 0;
        let mut num_polish = 0;
        let mut num_russian = 0;
        for resource in resources {
            let p = resource.path.to_lowercase().replace("\\", "/");
            if p.starts_with("dutch/") {
                num_dutch += 1;
            } else if p.starts_with("german/") {
                num_german += 1;
            } else if p.starts_with("italian/") {
                num_italian += 1;
            } else if p.starts_with("polish/") {
                num_polish += 1;
            } else if p.starts_with("russian/") {
                num_russian += 1;
            }
        }
        let mut best_num = 0;
        if num_dutch > 0 {
            info!("Found {} dutch resources", num_dutch);
            if best_num < num_dutch {
                best_num = num_dutch;
                best_version = Some(VanillaVersion::DUTCH);
            }
        }
        if num_german > 0 {
            info!("Found {} german resources", num_german);
            if best_num < num_german {
                best_num = num_german;
                best_version = Some(VanillaVersion::GERMAN);
            }
        }
        if num_italian > 0 {
            info!("Found {} italian resources", num_italian);
            if best_num < num_italian {
                best_num = num_italian;
                best_version = Some(VanillaVersion::ITALIAN);
            }
        }
        if num_polish > 0 {
            info!("Found {} polish resources", num_polish);
            if best_num < num_polish {
                best_num = num_polish;
                best_version = Some(VanillaVersion::POLISH);
            }
        }
        if num_russian > 0 {
            info!("Found {} russian resources", num_russian);
            if best_num < num_russian {
                best_num = num_russian;
                best_version = Some(VanillaVersion::RUSSIAN);
            }
        }
        if best_num > 0 {
            let version = best_version.unwrap();
            info!("Assuming vanilla_version {:?}", version);
            self.vanilla_version = best_version;
            Ok(())
        } else {
            Err("Give up".to_owned().into())
        }
    }

    fn compare_pack(
        &mut self,
        datadir: &Path,
        path: &Path,
    ) -> GuessResult<(VanillaVersion, Vec<Resource>)> {
        let pack = self.get_pack(&path)?;
        let version = self.get_version(&pack)?;
        let extra = self.match_resources(&pack, &datadir)?;
        Ok((version, extra))
    }

    fn get_datadir(&mut self, gamedir: &Path) -> GuessResult<PathBuf> {
        info!("Looking for data dir in {:?}", &gamedir);
        let data_caseless = Nfc::caseless("data");
        let mut paths: Vec<PathBuf> = gamedir
            .read_dir()?
            .filter_map(|x| {
                if let Ok(entry) = x {
                    if let Some(file_name) = entry.file_name().to_str() {
                        if Nfc::caseless(file_name) == data_caseless {
                            let path = entry.path();
                            if path.is_dir() {
                                return Some(path);
                            }
                        }
                    }
                }
                None
            })
            .collect();
        if paths.len() > 1 {
            return Err(format!("Too many data dirs: {:?}", paths).into());
        }
        if paths.len() == 0 {
            return Err("Data dir not found".to_owned().into());
        }
        let path = paths.remove(0);
        info!("Found {:?}", &path);
        Ok(path)
    }

    fn get_pack_paths(&mut self) -> GuessResult<Vec<PathBuf>> {
        let dir = Path::new("externalized/resource_packs");
        info!("Searching for resource packs in {:?}", &dir);
        let paths: Vec<PathBuf> = dir
            .read_dir()?
            .filter_map(|x| {
                if let Ok(entry) = x {
                    let path = entry.path();
                    if is_json_file(&path) {
                        info!("Found {:?}", &path);
                        return Some(path);
                    }
                }
                None
            })
            .collect();
        Ok(paths)
    }

    fn get_pack(&mut self, path: &Path) -> GuessResult<ResourcePack> {
        info!("Reading {:?}", &path);
        let f = File::open(&path)?;
        let mut pack: ResourcePack = serde_json::from_reader(f)?;
        pack.resources = sorted_resources(pack.resources);
        info!("Got resource pack {:?}", &pack.name);
        Ok(pack)
    }

    fn get_version(&mut self, pack: &ResourcePack) -> GuessResult<VanillaVersion> {
        if let Some(version) = pack.get_str("vanilla_version") {
            let version = VanillaVersion::from_str(version)?;
            info!("Found vanilla_version {:?}", &version);
            return Ok(version);
        }
        Err("vanilla_version is missing".to_owned().into())
    }

    fn match_resources(
        &mut self,
        pack: &ResourcePack,
        datadir: &Path,
    ) -> GuessResult<Vec<Resource>> {
        // Compare without hashes (fast)
        info!("Getting resources (no hashes)");
        let mut builder = ResourcePackBuilder::new();
        builder.with_path(&datadir, &datadir);
        let mut hashes = Vec::new();
        for (k, v) in pack.properties().iter() {
            if v.as_bool() != Some(true) {
                continue;
            }
            if k == "with_file_size" {
                info!("with_file_size");
                builder.with_file_size();
            } else if k.starts_with("with_archive_") {
                info!("{}", &k);
                builder.with_archive(&k["with_archive_".len()..]);
            } else if k.starts_with("with_hash_") {
                hashes.push(k["with_hash_".len()..].to_owned());
            }
        }
        let resources = sorted_resources(builder.clone().execute("no hashes")?.resources);
        info!("Found {} resources (no hashes)", resources.len());
        info!("Comparing resources (no hashes)");
        let mut available = resources.iter().fuse().peekable();
        let mut extras: Vec<Resource> = Vec::new();
        for want in pack.resources.iter() {
            let mut found = false;
            while let Some(have) = available.next() {
                found = have.path == want.path
                    && have.get_str("archive_path") == want.get_str("archive_path")
                    && have.get_i64("file_size") == want.get_i64("file_size");
                if found {
                    break;
                }
                if hashes.len() == 0 {
                    extras.push(have.to_owned());
                }
            }
            if !found && available.peek().is_none() {
                return Err(format!("resource {:?} not found", want.path).into());
            }
        }
        if hashes.len() == 0 {
            while let Some(have) = available.next() {
                extras.push(have.to_owned());
            }
            info!("Passed (no hashes) with {} extra resources", extras.len());
            return Ok(extras);
        }
        info!("Passed (no hashes), repeating with hashes");
        // Compare with hashes (slow)
        info!("Getting resources");
        for hash in &hashes {
            info!("with_hash_{}", &hash);
            builder.with_hash(hash);
        }
        let resources = sorted_resources(builder.execute("hashes")?.resources);
        info!("Found {} resources", resources.len());
        info!("Comparing resources");
        let mut available = resources.iter().fuse().peekable();
        for want in pack.resources.iter() {
            let mut found = false;
            while let Some(have) = available.next() {
                found = have.path == want.path
                    && have.get_str("archive_path") == want.get_str("archive_path")
                    && have.get_i64("file_size") == want.get_i64("file_size");
                for hash in &hashes {
                    let prop = "hash_".to_owned() + hash;
                    found = found && have.get_str(&prop) == want.get_str(&prop);
                }
                if found {
                    break;
                }
                extras.push(have.to_owned());
            }
            if !found && available.peek().is_none() {
                return Err(format!("resource {:?} not found", want.path).into());
            }
        }
        while let Some(have) = available.next() {
            extras.push(have.to_owned());
        }
        info!("Passed with {} extra resources", extras.len());
        return Ok(extras);
    }
}

fn is_json_file(path: &Path) -> bool {
    if !path.is_file() {
        return false;
    }
    return path.extension().unwrap_or(&OsString::new()).to_str() == Some("json");
}

fn sorted_resources(mut resources: Vec<Resource>) -> Vec<Resource> {
    resources.sort_unstable_by(|a, b| {
        if a.path == b.path {
            let a_path = a.get_str("archive_path").unwrap_or("");
            let b_path = b.get_str("archive_path").unwrap_or("");
            a_path.cmp(b_path)
        } else {
            a.path.cmp(&b.path)
        }
    });
    return resources;
}

#[derive(Debug)]
struct GuessError {
    desc: String,
}

impl Error for GuessError {
    fn description(&self) -> &str {
        return &self.desc;
    }
}

impl fmt::Display for GuessError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "GuessError({})", self.description())
    }
}

impl From<String> for GuessError {
    fn from(desc: String) -> Self {
        GuessError { desc }
    }
}

impl From<io::Error> for GuessError {
    fn from(err: io::Error) -> Self {
        GuessError {
            desc: format!("{:?}", err),
        }
    }
}

impl From<ResourceError> for GuessError {
    fn from(err: ResourceError) -> Self {
        GuessError {
            desc: format!("{:?}", err),
        }
    }
}

impl From<serde_json::Error> for GuessError {
    fn from(err: serde_json::Error) -> Self {
        GuessError {
            desc: format!("{:?}", err),
        }
    }
}
