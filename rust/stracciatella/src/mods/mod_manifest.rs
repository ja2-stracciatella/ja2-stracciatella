use serde::{Deserialize, Serialize};

/// JSON representation of a mod manifest
#[derive(Debug, Serialize, Deserialize)]
pub struct ModManifestJson {
    pub name: String,
    pub description: Option<String>,
    pub version: String,
}
