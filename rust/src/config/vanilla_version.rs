use std::str::FromStr;
use std::fmt;
use std::fmt::Display;
use serde::Deserialize;
use serde::Serialize;

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