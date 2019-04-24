use std::error::Error;
use std::path::Path;
use std::str::FromStr;
use std::fmt;
use std::fmt::Display;
use serde::Deserialize;
use serde::Serialize;

use crate::res::ResourcePack;

#[derive(Debug, PartialEq, Copy, Clone, Serialize, Deserialize)]
#[repr(C)]
#[allow(non_camel_case_types)]
pub enum VanillaVersion {
    DUTCH,
    ENGLISH,
    FRENCH,
    GERMAN,
    ITALIAN,
    POLISH,
    RUSSIAN,
    RUSSIAN_GOLD,
}

impl VanillaVersion {
    // Guess the version from the contents of the game dir.
    pub fn from_game_dir(dir: &Path) -> Result<Self, String> {
        // find data dir (best effort)
        for entry in dir.read_dir().map_err(|err| format!("Error reading game dir: {}", err.description()))? {
            let entry = entry.map_err(|err| format!("Error reading game dir entry: {}", err.description()))?;
            let path = entry.path();
            if path.is_dir() {
                if let Some(file_name) = path.file_name() {
                    if file_name.to_string_lossy().to_uppercase() == "DATA" {
                        // generate resource pack of data dir and try to guess
                        let mut pack = ResourcePack::default();
                        pack.set_property("with_archive_slf", true);
                        pack.add_dir(&path).map_err(|err| format!("Error reading data dir: {}", err.description()))?;
                        for resource in pack.resources.iter() {
                            // guess version from the resource path
                            if let Some(version) = Self::from_resource_path(&resource.path) {
                                return Ok(version);
                            }
                        }
                        break;
                    }
                }
            }
        }
        return Err(format!("Data dir not found"))
    }

    // Guess the version from the resource path.
    pub fn from_resource_path(resource_path: &str) -> Option<Self> {
        let resource_path = resource_path.to_uppercase().replace("\\", "/");
        if resource_path.starts_with("GERMAN/") {
            return Some(VanillaVersion::GERMAN);
        }
        if resource_path.starts_with("DUTCH/") {
            return Some(VanillaVersion::DUTCH);
        }
        if resource_path.starts_with("ITALIAN/") {
            return Some(VanillaVersion::ITALIAN);
        }
        if resource_path.starts_with("POLISH/") {
            return Some(VanillaVersion::POLISH);
        }
        if resource_path.starts_with("RUSSIAN/") {
            return Some(VanillaVersion::RUSSIAN);
        }
        return None;
    }
}

impl FromStr for VanillaVersion {
    type Err = String;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match s {
            "DUTCH" => Ok(VanillaVersion::DUTCH),
            "ENGLISH" => Ok(VanillaVersion::ENGLISH),
            "FRENCH" => Ok(VanillaVersion::FRENCH),
            "GERMAN" => Ok(VanillaVersion::GERMAN),
            "ITALIAN" => Ok(VanillaVersion::ITALIAN),
            "POLISH" => Ok(VanillaVersion::POLISH),
            "RUSSIAN" => Ok(VanillaVersion::RUSSIAN),
            "RUSSIAN_GOLD" => Ok(VanillaVersion::RUSSIAN_GOLD),
            _ => Err(format!("Resource version {} is unknown", s))
        }
    }
}

impl Display for VanillaVersion {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", match self {
            VanillaVersion::DUTCH => "Dutch",
            VanillaVersion::ENGLISH => "English",
            VanillaVersion::FRENCH => "French",
            VanillaVersion::GERMAN => "German",
            VanillaVersion::ITALIAN => "Italian",
            VanillaVersion::POLISH => "Polish",
            VanillaVersion::RUSSIAN => "Russian",
            VanillaVersion::RUSSIAN_GOLD => "Russian (Gold)",
        })
    }
}