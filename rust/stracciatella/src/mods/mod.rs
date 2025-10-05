use std::collections::HashMap;
use std::io;
use std::path::{Path, PathBuf};

mod mod_manifest;

use mod_manifest::ModManifestJson;
use regex::Regex;

use crate::config::EngineOptions;

/// Path to a mod
#[derive(Debug, Clone)]
pub enum ModPath {
    Path(PathBuf),
    #[cfg(target_os = "android")]
    AndroidAssetPath(PathBuf),
}

impl ModPath {
    /// Joins the mod path with another path
    pub fn join<P: AsRef<Path>>(&self, p: P) -> ModPath {
        match self {
            ModPath::Path(s) => ModPath::Path(s.join(p)),
            #[cfg(target_os = "android")]
            ModPath::AndroidAssetPath(s) => ModPath::AndroidAssetPath(s.join(p)),
        }
    }

    /// Extracts the mod id from the mod path
    pub fn id(&self) -> std::io::Result<String> {
        let dir_name = match self {
            ModPath::Path(s) => s,
            #[cfg(target_os = "android")]
            ModPath::AndroidAssetPath(s) => s,
        }
        .file_name()
        .ok_or_else(|| {
            io::Error::new(
                io::ErrorKind::InvalidData,
                format!("Mod path {:?} does not contain dir name", self),
            )
        })?;
        let dir_name = dir_name.to_string_lossy();

        let mod_id_regex = Regex::new(r"^[a-z0-9\-]+$").unwrap();
        if mod_id_regex.is_match(&dir_name) {
            Ok(dir_name.into())
        } else {
            Err(io::Error::new(
                io::ErrorKind::InvalidData,
                format!(
                    "Mod directory {:?} does conform to mod id format: {}",
                    self, mod_id_regex
                ),
            ))
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
    /// Create a mod instance from a path on disk
    pub fn from_path(path: &Path) -> io::Result<Self> {
        let manifest_path =
            crate::fs::resolve_existing_components(Path::new("manifest.json"), Some(path), true);
        let path = ModPath::Path(path.to_owned());
        let id = path.id()?;

        match std::fs::read_to_string(&manifest_path) {
            Ok(s) => match crate::json::de::from_string::<ModManifestJson>(&s) {
                Ok(json) => Ok(Mod::new_with_mod_manifest(path, id, json)),
                Err(e) => {
                    log::warn!(
                        "Could not read mod manifest for `{}` at location `{:?}`: {}",
                        id,
                        manifest_path,
                        e
                    );
                    Ok(Mod::new_with_id(path, id))
                }
            },
            Err(e) => {
                log::warn!(
                    "Could not read mod manifest for `{}` at location `{:?}`: {}",
                    id,
                    manifest_path,
                    e
                );
                Ok(Mod::new_with_id(path, id))
            }
        }
    }

    /// Create a mod instance from an android assets path
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
        let path = ModPath::AndroidAssetPath(path.to_owned());
        let id = path.id()?;

        if let Some(mut file) = asset_manager.open(&manifest_path_cstring) {
            let mut s = Default::default();
            file.read_to_string(&mut s)?;
            match crate::json::de::from_string::<ModManifestJson>(&s) {
                Ok(json) => Ok(Mod::new_with_mod_manifest(path, id, json)),
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

    /// Create minimal mod with only id (used as fallback for mods without manifests)
    fn new_with_id(path: ModPath, id: String) -> Self {
        Mod {
            id,
            name: None,
            description: None,
            version: None,
            path,
        }
    }

    /// Create mod from a json manifest
    fn new_with_mod_manifest(path: ModPath, id: String, json: ModManifestJson) -> Self {
        Mod {
            id,
            name: Some(json.name),
            description: json.description,
            version: Some(json.version),
            path,
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

/// The ModManager manages installed mods and provides mod descriptions and paths for them
#[derive(Debug, Clone)]
pub struct ModManager {
    available_mods: Vec<Mod>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ModManagerInitError {
    /// There are some mods enabled that dont exist on disk
    MissingEnabledMods(Vec<String>),
}

impl std::fmt::Display for ModManagerInitError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "ModManager: Error initializing mods. ")?;
        match self {
            ModManagerInitError::MissingEnabledMods(v) => write!(
                f,
                "The following mods are enabled, but missing on your filesystem: {}",
                v.join(", ")
            )?,
        };

        Ok(())
    }
}

impl std::error::Error for ModManagerInitError {}

impl ModManager {
    /// Create a mod manager. Might return an error, e.g. when a mod is enabled that does not exist on disk.
    pub fn new(engine_options: &EngineOptions) -> Result<ModManager, ModManagerInitError> {
        let mod_manager = Self::new_unchecked(engine_options);

        let missing_mods: Vec<_> = engine_options
            .mods
            .iter()
            .filter(|v| mod_manager.get_mod_by_id(v).is_none())
            .collect();
        if !missing_mods.is_empty() {
            Err(ModManagerInitError::MissingEnabledMods(
                missing_mods.into_iter().cloned().collect(),
            ))
        } else {
            Ok(mod_manager)
        }
    }

    /// Creates a mod manager without doing any sanity checks
    pub fn new_unchecked(engine_options: &EngineOptions) -> ModManager {
        let mods_dir = Path::new("mods");
        let home_dir = crate::fs::resolve_existing_components(
            mods_dir,
            Some(&engine_options.stracciatella_home),
            true,
        );
        #[cfg(not(target_os = "android"))]
        let assets_dir = crate::fs::resolve_existing_components(
            mods_dir,
            Some(&engine_options.assets_dir),
            true,
        );
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
                                log::debug!("Found mod `{}` at `{:?}`", m.id(), m.path());
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
                    if let Ok(false) = entry.as_ref().map(|e| e.path().is_dir()) {
                        continue;
                    }
                    match entry.and_then(|dir_entry| Mod::from_path(&dir_entry.path())) {
                        Ok(m) => {
                            log::debug!("Found mod `{}` at `{:?}`", m.id(), m.path());
                            available_mods.insert(m.id().to_owned(), m);
                        }
                        Err(e) => {
                            log::warn!("Error loading mod directory: {}", e);
                        }
                    }
                }
            }
        }

        let mut available_mods: Vec<_> = available_mods.into_values().collect();
        available_mods.sort_by_key(|v1| v1.name().to_lowercase());

        ModManager { available_mods }
    }

    pub fn available_mods(&self) -> &[Mod] {
        &self.available_mods
    }

    pub fn get_mod_by_id(&self, id: &str) -> Option<&Mod> {
        self.available_mods.iter().find(|m| m.id() == id)
    }
}

#[cfg(test)]
mod tests {
    use tempfile::{TempDir, tempdir};

    use crate::{
        config::EngineOptions,
        mods::{ModManagerInitError, mod_manifest::ModManifestJson},
    };

    use super::ModManager;

    #[test]
    fn missing_mod_should_fail() {
        let (mut engine_options, _temp_dir) = create_test_engine_options();
        engine_options.mods = vec!["test-mod".to_owned()];
        assert_eq!(
            ModManager::new(&engine_options).unwrap_err(),
            ModManagerInitError::MissingEnabledMods(vec!["test-mod".to_owned()])
        );
    }

    #[test]
    fn mod_with_invalid_name_should_be_ignored() {
        let (engine_options, _temp_dir) = create_test_engine_options();

        std::fs::create_dir_all(engine_options.stracciatella_home.join("mods/TEST-MOD-1"))
            .expect("create dir `mods/TEST-MOD-1`");
        std::fs::create_dir_all(engine_options.stracciatella_home.join("mods/test-mod-2"))
            .expect("create dir `mods/test-mod-2`");
        std::fs::create_dir_all(engine_options.assets_dir.join("mods/test-mod-Ω"))
            .expect("create dir `mods/test-mod-Ω`");

        let mod_manager = ModManager::new(&engine_options).unwrap();
        let mod_names: Vec<_> = mod_manager
            .available_mods()
            .iter()
            .map(|m| m.name().to_owned())
            .collect();

        assert_eq!(mod_names, vec!["test-mod-2".to_owned()]);
    }

    #[test]
    fn reading_available_mods_should_be_case_insensitive() {
        let (engine_options, _temp_dir) = create_test_engine_options();

        std::fs::create_dir_all(engine_options.stracciatella_home.join("mods/test-mod-1"))
            .expect("create dir `mods/test-mod-1`");
        std::fs::create_dir_all(engine_options.assets_dir.join("mOdS/test-mod-2"))
            .expect("create dir `mods/test-mod-2`");

        let mod_manager = ModManager::new(&engine_options).unwrap();
        let mod_names: Vec<_> = mod_manager
            .available_mods()
            .iter()
            .map(|m| m.name().to_owned())
            .collect();

        assert_eq!(
            mod_names,
            vec!["test-mod-1".to_owned(), "test-mod-2".to_owned()]
        );
    }

    #[test]
    fn mod_without_manifest_should_load() {
        let (engine_options, _temp_dir) = create_test_engine_options();

        std::fs::create_dir_all(engine_options.stracciatella_home.join("mods/test-mod-1"))
            .expect("create dir `mods/test-mod-1`");

        let mod_manager = ModManager::new(&engine_options).unwrap();
        let m = mod_manager.get_mod_by_id("test-mod-1").unwrap();

        assert_eq!(m.name(), "test-mod-1");
        assert_eq!(m.version(), "unknown");
        assert_eq!(m.description(), "");
    }

    #[test]
    fn mod_with_invalid_manifest_should_load() {
        let (engine_options, _temp_dir) = create_test_engine_options();

        std::fs::create_dir_all(engine_options.stracciatella_home.join("mods/test-mod-1"))
            .expect("create dir `mods/test-mod-1`");
        std::fs::write(
            engine_options
                .stracciatella_home
                .join("mods/test-mod-1/manifest.json"),
            "foobar",
        )
        .expect("write manifest");

        let mod_manager = ModManager::new(&engine_options).unwrap();
        let m = mod_manager.get_mod_by_id("test-mod-1").unwrap();

        assert_eq!(m.name(), "test-mod-1");
        assert_eq!(m.version(), "unknown");
        assert_eq!(m.description(), "");
    }

    #[test]
    fn mod_with_manifest_should_load() {
        let (engine_options, _temp_dir) = create_test_engine_options();
        let manifest = ModManifestJson {
            name: "m1".to_owned(),
            description: Some("test description".to_owned()),
            version: "1.0.0".to_owned(),
        };
        let json = serde_json::to_string(&manifest).expect("serialization");

        std::fs::create_dir_all(engine_options.stracciatella_home.join("mods/test-mod-1"))
            .expect("create dir `mods/test-mod-1`");
        std::fs::write(
            engine_options
                .stracciatella_home
                .join("mods/test-mod-1/manifest.json"),
            &json,
        )
        .expect("write manifest");

        let mod_manager = ModManager::new(&engine_options).unwrap();
        let m = mod_manager.get_mod_by_id("test-mod-1").unwrap();

        assert_eq!(m.name(), "m1");
        assert_eq!(m.version(), "1.0.0");
        assert_eq!(m.description(), "test description");
    }

    fn create_test_engine_options() -> (EngineOptions, TempDir) {
        let temp_dir = tempdir().expect("temp_dir");
        let mut engine_options = EngineOptions::default();

        let home_dir = temp_dir.path().join("home");
        let assets_dir = temp_dir.path().join("assets");

        std::fs::create_dir(&home_dir).expect("home_dir");
        std::fs::create_dir_all(assets_dir.join("externalized")).expect("assets_dir");

        engine_options.stracciatella_home = home_dir;
        engine_options.assets_dir = assets_dir;

        (engine_options, temp_dir)
    }
}
