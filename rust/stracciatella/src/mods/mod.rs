use std::collections::HashMap;
use std::io;
use std::path::{Path, PathBuf};

mod mod_manifest;

use mod_manifest::ModManifestJson;

/// Information about a mod
#[derive(Debug, Clone)]
pub struct Mod {
    id: String,
    name: Option<String>,
    description: Option<String>,
    version: Option<String>,
    path: PathBuf,
}

impl Mod {
    pub fn from_path(path: &Path) -> io::Result<Self> {
        let invalid_mod_id = || {
            io::Error::new(
                io::ErrorKind::InvalidData,
                "Directory name is not a valid mod id".to_owned(),
            )
        };
        let id = path
            .file_name()
            .map(|f| f.to_string_lossy())
            .ok_or_else(invalid_mod_id)?;
        if id.contains(std::char::REPLACEMENT_CHARACTER) {
            return Err(invalid_mod_id());
        }

        let manifest_path =
            crate::fs::resolve_existing_components(Path::new("manifest.json"), Some(path), true);
        match std::fs::read_to_string(&manifest_path) {
            Ok(s) => match crate::json::de::from_string::<ModManifestJson>(&s) {
                Ok(json) => Ok(Mod::new_with_mod_manifest(path, json)),
                Err(e) => {
                    log::warn!(
                        "Could not read mod manifest for `{}` at location `{:?}`: {}",
                        id,
                        manifest_path,
                        e
                    );
                    Ok(Mod::new_with_id(path, &id))
                }
            },
            Err(e) => {
                log::warn!(
                    "Could not read mod manifest for `{}` at location `{:?}`: {}",
                    id,
                    manifest_path,
                    e
                );
                Ok(Mod::new_with_id(path, &id))
            }
        }
    }

    fn new_with_id(path: &Path, id: &str) -> Self {
        Mod {
            id: id.to_owned(),
            name: None,
            description: None,
            version: None,
            path: path.to_owned(),
        }
    }

    fn new_with_mod_manifest(path: &Path, json: ModManifestJson) -> Self {
        Mod {
            id: json.id,
            name: Some(json.name),
            description: json.description,
            version: Some(json.version),
            path: path.to_owned(),
        }
    }

    pub fn id(&self) -> &str {
        &self.id
    }

    pub fn name(&self) -> &str {
        self.name.as_deref().unwrap_or(&self.id)
    }

    pub fn description(&self) -> &str {
        self.description.as_deref().unwrap_or_default()
    }

    pub fn version(&self) -> &str {
        self.version.as_deref().unwrap_or("unknown")
    }

    pub fn path(&self) -> &Path {
        &self.path
    }
}

#[derive(Debug, Clone)]
pub struct ModManager {
    available_mods: Vec<Mod>,
}

impl ModManager {
    pub fn new(home_dir: &Path, assets_dir: &Path) -> ModManager {
        let home_dir =
            crate::fs::resolve_existing_components(Path::new("mods"), Some(home_dir), true);
        let assets_dir =
            crate::fs::resolve_existing_components(Path::new("mods"), Some(assets_dir), true);
        let mut available_mods = HashMap::new();

        for dir in &[&assets_dir, &home_dir] {
            if let Ok(paths) = dir.read_dir() {
                for entry in paths {
                    if let Some(false) = entry.as_ref().map(|e| e.path().is_dir()).ok() {
                        continue;
                    }
                    match entry.and_then(|dir_entry| Mod::from_path(&dir_entry.path())) {
                        Ok(manifest) => {
                            log::debug!("Found mod: {:?}", manifest);
                            available_mods.insert(manifest.id().to_owned(), manifest);
                        }
                        Err(e) => {
                            log::warn!("Error loading mod directory: {}", e);
                        }
                    }
                }
            }
        }

        let mut available_mods: Vec<_> = available_mods.into_iter().map(|(_, v)| v).collect();
        available_mods.sort_by(|v1, v2| v1.name().to_lowercase().cmp(&v2.name().to_lowercase()));
        log::info!(
            "{:?}",
            available_mods.iter().map(|v| v.name()).collect::<Vec<_>>()
        );
        ModManager { available_mods }
    }

    pub fn available_mods(&self) -> &[Mod] {
        &self.available_mods
    }

    pub fn get_mod_by_id(&self, id: &str) -> Option<&Mod> {
        self.available_mods.iter().find(|m| m.id() == id)
    }
}
