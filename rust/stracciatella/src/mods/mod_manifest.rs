use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize)]
pub struct ModManifestJson {
    pub id: String,
    pub name: String,
    pub description: Option<String>,
    pub version: String,
}
