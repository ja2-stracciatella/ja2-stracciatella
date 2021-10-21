use std::collections::HashMap;
use std::io;
use std::path::{Path, PathBuf};

mod mod_manifest;

use mod_manifest::ModManifestJson;

#[derive(Debug, Clone)]
pub enum ModPath {
    Path(PathBuf),
    #[cfg(target_os = "android")]
    AndroidAssetPath(PathBuf),
}

impl ModPath {
    pub fn join<P: AsRef<Path>>(&self, p: P) -> ModPath {
        match self {
            ModPath::Path(s) => ModPath::Path(s.join(p)),
            #[cfg(target_os = "android")]
            ModPath::AndroidAssetPath(s) => ModPath::AndroidAssetPath(s.join(p)),
        }
    }
}

/// Information about a mod
#[derive(Debug, Clone)]
pub struct Mod {
    id: String,
    name: Option<String>,
    description: Option<String>,
    version: Option<String>,
    path: ModPath,
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
                Ok(json) => Ok(Mod::new_with_mod_manifest(
                    ModPath::Path(path.to_owned()),
                    json,
                )),
                Err(e) => {
                    log::warn!(
                        "Could not read mod manifest for `{}` at location `{:?}`: {}",
                        id,
                        manifest_path,
                        e
                    );
                    Ok(Mod::new_with_id(ModPath::Path(path.to_owned()), &id))
                }
            },
            Err(e) => {
                log::warn!(
                    "Could not read mod manifest for `{}` at location `{:?}`: {}",
                    id,
                    manifest_path,
                    e
                );
                Ok(Mod::new_with_id(ModPath::Path(path.to_owned()), &id))
            }
        }
    }

    #[cfg(target_os = "android")]
    pub fn from_android_assets(path: &Path) -> io::Result<Self> {
        use std::ffi::CString;
        use std::io::Read;

        let manifest_path = path.join("manifest.json");
        let asset_manager = crate::android::get_asset_manager().map_err(|e| {
            io::Error::new(
                io::ErrorKind::Other,
                format!("Could not get asset manager: {}", e),
            )
        })?;
        let manifest_path_cstring = CString::new(manifest_path.to_string_lossy().as_bytes())
            .expect("should not contain 0 byte");

        if let Some(mut file) = asset_manager.open(&manifest_path_cstring) {
            let mut s = Default::default();
            file.read_to_string(&mut s)?;
            match crate::json::de::from_string::<ModManifestJson>(&s) {
                Ok(json) => Ok(Mod::new_with_mod_manifest(
                    ModPath::Path(path.to_owned()),
                    json,
                )),
                Err(e) => Err(io::Error::new(
                    io::ErrorKind::NotFound,
                    format!(
                        "Mod manifest `{:?}` could not be read in android asset fs: {}",
                        manifest_path, e
                    ),
                )),
            }
        } else {
            Err(io::Error::new(
                io::ErrorKind::NotFound,
                format!(
                    "Mod manifest `{:?}` not found in android asset fs",
                    manifest_path
                ),
            ))
        }
    }

    fn new_with_id(path: ModPath, id: &str) -> Self {
        Mod {
            id: id.to_owned(),
            name: None,
            description: None,
            version: None,
            path: path.clone(),
        }
    }

    fn new_with_mod_manifest(path: ModPath, json: ModManifestJson) -> Self {
        Mod {
            id: json.id,
            name: Some(json.name),
            description: json.description,
            version: Some(json.version),
            path: path.clone(),
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

    pub fn path(&self) -> &ModPath {
        &self.path
    }
}

#[derive(Debug, Clone)]
pub struct ModManager {
    available_mods: Vec<Mod>,
}

impl ModManager {
    pub fn new(home_dir: &Path, assets_dir: &Path) -> ModManager {
        let mods_dir = Path::new("mods");
        let home_dir = crate::fs::resolve_existing_components(mods_dir, Some(home_dir), true);
        #[cfg(not(target_os = "android"))]
        let assets_dir = crate::fs::resolve_existing_components(mods_dir, Some(assets_dir), true);
        let mut available_mods = HashMap::new();

        #[cfg(target_os = "android")]
        let dirs = [home_dir];
        #[cfg(not(target_os = "android"))]
        let dirs = [assets_dir, home_dir];

        #[cfg(target_os = "android")]
        {
            // Android special case. List mods from assetfs. All stracciatella included mods MUST provide a `manifest.json`.
            match crate::android::list_asset_dir(&mods_dir) {
                Ok(paths) => {
                    for path in paths {
                        let path = mods_dir.join(path);
                        match Mod::from_android_assets(&path) {
                            Ok(m) => {
                                log::debug!("Found mod: {:?}", m);
                                available_mods.insert(m.id().to_owned(), m);
                            }
                            Err(e) => {
                                log::warn!("Error loading mod directory: {}", e);
                            }
                        }
                    }
                }
                Err(e) => log::error!("Could not list mods directory in android asset fs"),
            }
        }

        for dir in &dirs {
            if let Ok(paths) = dir.read_dir() {
                for entry in paths {
                    if let Some(false) = entry.as_ref().map(|e| e.path().is_dir()).ok() {
                        continue;
                    }
                    match entry.and_then(|dir_entry| Mod::from_path(&dir_entry.path())) {
                        Ok(m) => {
                            log::debug!("Found mod: {:?}", m);
                            available_mods.insert(m.id().to_owned(), m);
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
