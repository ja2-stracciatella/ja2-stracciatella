use std::default::Default;
use std::fmt;
use std::fmt::Display;
use std::str::FromStr;

use serde::{Deserialize, Deserializer, Serialize, Serializer};

/// Struct that contains a specific resolution for the game
#[derive(Debug, PartialEq, Eq, Copy, Clone)]
pub struct Resolution(pub u16, pub u16);

impl FromStr for Resolution {
    type Err = String;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let mut resolutions = s.split('x').filter_map(|r_str| r_str.parse::<u16>().ok());

        match (resolutions.next(), resolutions.next()) {
            (Some(x), Some(y)) => Ok(Resolution(x, y)),
            _ => Err(String::from(
                "Incorrect resolution format, should be WIDTHxHEIGHT.",
            )),
        }
    }
}

impl Display for Resolution {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}x{}", self.0, self.1)
    }
}

impl Serialize for Resolution {
    fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        serializer.serialize_str(&format!("{}", self))
    }
}

impl<'de> Deserialize<'de> for Resolution {
    fn deserialize<D>(deserializer: D) -> Result<Self, D::Error>
    where
        D: Deserializer<'de>,
    {
        let deserialized_string = String::deserialize(deserializer)?;
        Resolution::from_str(&deserialized_string).map_err(serde::de::Error::custom)
    }
}

impl Default for Resolution {
    fn default() -> Self {
        Resolution(640, 480)
    }
}
