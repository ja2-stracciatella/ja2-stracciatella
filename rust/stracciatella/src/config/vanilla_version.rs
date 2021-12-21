use std::fmt;
use std::fmt::Display;
use std::str::FromStr;

use serde::{Deserialize, Serialize};

/// Enum for the vanilla game version that is used to run the game
#[derive(Debug, PartialEq, Copy, Clone, Serialize, Deserialize)]
#[repr(C)]
#[allow(non_camel_case_types)]
pub enum VanillaVersion {
    // Dutch Version
    DUTCH,
    // English Version
    ENGLISH,
    // French Version
    FRENCH,
    // German Version
    GERMAN,
    // Italian Version
    ITALIAN,
    // Polish Version
    POLISH,
    // Russian Version (“BUKA Agonia Vlasty” release)
    RUSSIAN,
    // Russian Version ("Gold" release)
    RUSSIAN_GOLD,
    // CHINESE Version
    SIMPLIFIED_CHINESE,
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
            "SIMPLIFIED_CHINESE" => Ok(VanillaVersion::SIMPLIFIED_CHINESE),
            _ => Err(format!("Resource version {} is unknown", s)),
        }
    }
}

impl Display for VanillaVersion {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "{}",
            match self {
                VanillaVersion::DUTCH => "Dutch",
                VanillaVersion::ENGLISH => "English",
                VanillaVersion::FRENCH => "French",
                VanillaVersion::GERMAN => "German",
                VanillaVersion::ITALIAN => "Italian",
                VanillaVersion::POLISH => "Polish",
                VanillaVersion::RUSSIAN => "Russian",
                VanillaVersion::RUSSIAN_GOLD => "Russian (Gold)",
                VanillaVersion::SIMPLIFIED_CHINESE => "Simplified Chinese",
            }
        )
    }
}
