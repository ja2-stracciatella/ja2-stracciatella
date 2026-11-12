//! The settings from `game.json` that players can change in a launcher.
//!
//! The settings, their titles, descriptions and allowed values are taken from the json schema of
//! `game.json`. Settings that are not described there are not shown. The order of the settings
//! and their defaults are taken from `game.json` as patched by the enabled mods, so the defaults
//! are the values the game uses when the player has not changed anything.
//!
//! Changed values are written as a JSON patch (RFC 6902) to `game.patch.json` in the data dir
//! within stracciatella home. That is the highest priority VFS layer, so the game applies the
//! patch after `game.json` and all patches of mods.

use std::collections::HashMap;
use std::fs as stdfs;
use std::path::{Path, PathBuf};

use jsonschema::{Draft, Validator};
use log::{info, warn};
use serde_json::{Map, Number, Value, json};

use crate::config::EngineOptions;
use crate::fs;
use crate::json;
use crate::mods::ModManager;
use crate::schemas::SchemaManager;
use crate::unicode::Nfc;
use crate::vfs::Vfs;

const GAME_JSON: &str = "game.json";
const PATCH_FILE_NAME: &str = "game.patch.json";

/// The kind of value of a setting
#[derive(Debug, PartialEq, Eq, Copy, Clone)]
#[repr(C)]
pub enum GameSettingType {
    Boolean,
    Integer,
    Decimal,
    Text,
}

/// A single value in `game.json` that can be changed
#[derive(Debug)]
pub struct GameSetting {
    /// JSON pointer (RFC 6901) of the setting, as used in the patch file
    pointer: String,
    section: usize,
    title: String,
    description: Option<String>,
    allowed_values: Option<String>,
    setting_type: GameSettingType,
    default_value: Value,
    validator: Option<Validator>,
}

impl GameSetting {
    pub fn pointer(&self) -> &str {
        &self.pointer
    }

    /// Index of the section the setting belongs to
    pub fn section(&self) -> usize {
        self.section
    }

    pub fn title(&self) -> &str {
        &self.title
    }

    pub fn description(&self) -> Option<&str> {
        self.description.as_deref()
    }

    /// A text describing the allowed values, or None if any value of the type is allowed
    pub fn allowed_values(&self) -> Option<&str> {
        self.allowed_values.as_deref()
    }

    pub fn setting_type(&self) -> GameSettingType {
        self.setting_type
    }

    /// Parses the text representation of a value.
    /// Returns None if it is not a valid value for the setting according to its schema.
    pub fn parse(&self, input: &str) -> Option<Value> {
        let value = match self.setting_type {
            GameSettingType::Boolean => match input {
                "true" => Value::Bool(true),
                "false" => Value::Bool(false),
                _ => return None,
            },
            GameSettingType::Integer => Value::from(input.trim().parse::<i64>().ok()?),
            GameSettingType::Decimal => {
                let v = input.trim().parse::<f64>().ok()?;
                Value::Number(Number::from_f64(v)?)
            }
            GameSettingType::Text => Value::String(input.to_owned()),
        };
        self.is_valid(&value).then_some(value)
    }

    /// Whether a value read from a patch file can be used for the setting
    fn accepts(&self, value: &Value) -> bool {
        let type_matches = match self.setting_type {
            GameSettingType::Boolean => value.is_boolean(),
            GameSettingType::Integer => value.is_i64() || value.is_u64(),
            GameSettingType::Decimal => value.is_number(),
            GameSettingType::Text => value.is_string(),
        };
        type_matches && self.is_valid(value)
    }

    fn is_valid(&self, value: &Value) -> bool {
        self.validator.as_ref().is_none_or(|v| v.is_valid(value))
    }

    fn is_same_value(&self, a: &Value, b: &Value) -> bool {
        if self.setting_type == GameSettingType::Decimal {
            return a.as_f64() == b.as_f64();
        }
        a == b
    }
}

/// The text representation of a value, the reverse of `GameSetting::parse`
fn value_to_text(value: &Value) -> String {
    match value {
        Value::String(s) => s.clone(),
        v => v.to_string(),
    }
}

/// The settings from `game.json` and the values the player changed
#[derive(Debug)]
pub struct GameSettings {
    /// Titles of the sections, None for settings outside of a section
    sections: Vec<Option<String>>,
    settings: Vec<GameSetting>,
    patch_path: PathBuf,
    /// Values the player changed by the pointer of the setting
    overrides: HashMap<String, Value>,
    /// Patch operations that were not written by a launcher, kept as they are
    foreign_operations: Vec<Value>,
    /// Whether the patch file needs to be written
    dirty: bool,
}

impl GameSettings {
    /// Loads the settings with the defaults of the mods in the engine options and
    /// the values the player changed
    pub fn load(engine_options: &EngineOptions, mod_manager: &ModManager) -> Result<Self, String> {
        let game_json = read_game_json_without_player_patch(engine_options, mod_manager)?;
        let schema_manager = SchemaManager::default();
        let schema = schema_manager
            .get(Path::new(GAME_JSON))
            .ok_or_else(|| format!("no schema for {}", GAME_JSON))?
            .as_value();
        let (sections, settings) = build_settings(&game_json, schema);

        let patch_path = fs::resolve_existing_components(
            Path::new(PATCH_FILE_NAME),
            Some(&Vfs::home_data_dir(engine_options)),
            true,
        );
        let mut game_settings = GameSettings {
            sections,
            settings,
            patch_path,
            overrides: HashMap::new(),
            foreign_operations: Vec::new(),
            dirty: false,
        };
        game_settings.read_patch_file(&game_json);
        Ok(game_settings)
    }

    fn read_patch_file(&mut self, game_json: &Value) {
        let content = match stdfs::read_to_string(&self.patch_path) {
            Ok(content) => content,
            Err(e) if e.kind() == std::io::ErrorKind::NotFound => return,
            Err(e) => {
                // Do not overwrite a file that might be fine
                warn!("Could not read {:?}: {}", self.patch_path, e);
                return;
            }
        };
        let operations = match json::de::from_string::<Value>(&content) {
            Ok(Value::Array(operations)) => operations,
            _ => {
                // The game cannot start with an invalid patch, so replace it on the next save
                warn!(
                    "Could not decode {:?}, it will be replaced",
                    self.patch_path
                );
                self.dirty = true;
                return;
            }
        };

        let settings_by_pointer: HashMap<&str, &GameSetting> =
            self.settings.iter().map(|s| (s.pointer(), s)).collect();
        for operation in operations {
            let op = operation.get("op").and_then(Value::as_str);
            let path = operation.get("path").and_then(Value::as_str);
            let (op, path) = match (op, path) {
                (Some(op @ ("add" | "replace")), Some(path)) => (op, path),
                _ => {
                    self.foreign_operations.push(operation);
                    continue;
                }
            };
            let value = operation.get("value");
            if let Some(setting) = settings_by_pointer.get(path) {
                match value {
                    Some(value) if setting.accepts(value) => {
                        self.overrides.insert(path.to_owned(), value.clone());
                    }
                    _ => {
                        // The type or the allowed values of the setting changed since the value
                        // was saved. The value no longer applies, and could stop the game from starting.
                        info!(
                            "Dropping outdated value for {} from {:?}",
                            path, self.patch_path
                        );
                        self.dirty = true;
                    }
                }
            } else if would_apply(game_json, op, path) {
                self.foreign_operations.push(operation);
            } else {
                // The value was removed or moved since the operation was saved, applying it
                // would stop the game from starting
                info!(
                    "Dropping outdated value for {} from {:?}",
                    path, self.patch_path
                );
                self.dirty = true;
            }
        }
    }

    /// Titles of the sections, None for settings outside of a section
    pub fn sections(&self) -> &[Option<String>] {
        &self.sections
    }

    pub fn settings(&self) -> &[GameSetting] {
        &self.settings
    }

    /// Path of the patch file the changed values are written to
    pub fn patch_path(&self) -> &Path {
        &self.patch_path
    }

    pub fn value_of(&self, index: usize) -> &Value {
        let setting = &self.settings[index];
        self.overrides
            .get(setting.pointer())
            .unwrap_or(&setting.default_value)
    }

    /// The text representation of the current value of a setting
    pub fn value_text_of(&self, index: usize) -> String {
        value_to_text(self.value_of(index))
    }

    pub fn is_modified(&self, index: usize) -> bool {
        self.overrides.contains_key(self.settings[index].pointer())
    }

    /// Sets the value of a setting from its text representation.
    /// Returns false if it is not a valid value for the setting.
    pub fn set(&mut self, index: usize, input: &str) -> bool {
        let setting = &self.settings[index];
        let Some(value) = setting.parse(input) else {
            return false;
        };
        if setting.is_same_value(self.value_of(index), &value) {
            return true;
        }
        let setting = &self.settings[index];
        if setting.is_same_value(&setting.default_value, &value) {
            self.overrides.remove(setting.pointer());
        } else {
            self.overrides.insert(setting.pointer().to_owned(), value);
        }
        self.dirty = true;
        true
    }

    /// Resets all settings to their defaults
    pub fn reset_all(&mut self) {
        if !self.overrides.is_empty() {
            self.overrides.clear();
            self.dirty = true;
        }
    }

    /// Writes the changed values to the patch file, or deletes it if nothing is left to patch
    pub fn save(&mut self) -> Result<(), String> {
        if !self.dirty {
            return Ok(());
        }
        let mut operations = self.foreign_operations.clone();
        for setting in &self.settings {
            if let Some(value) = self.overrides.get(setting.pointer()) {
                // add replaces existing members, but unlike replace does not fail if a mod's
                // game.json lacks the member
                operations.push(json!({ "op": "add", "path": setting.pointer(), "value": value }));
            }
        }
        if operations.is_empty() {
            if self.patch_path.exists() {
                stdfs::remove_file(&self.patch_path)
                    .map_err(|e| format!("Could not delete {:?}: {}", self.patch_path, e))?;
            }
        } else {
            if let Some(parent) = self.patch_path.parent() {
                stdfs::create_dir_all(parent)
                    .map_err(|e| format!("Could not create {:?}: {}", parent, e))?;
            }
            let content = serde_json::to_string_pretty(&Value::Array(operations))
                .map_err(|e| e.to_string())?;
            stdfs::write(&self.patch_path, content + "\n")
                .map_err(|e| format!("Could not write {:?}: {}", self.patch_path, e))?;
        }
        self.dirty = false;
        Ok(())
    }
}

/// Reads game.json as the game would, but without the patch of the player.
/// Enabled mods that do not exist are skipped.
fn read_game_json_without_player_patch(
    engine_options: &EngineOptions,
    mod_manager: &ModManager,
) -> Result<Value, String> {
    let mut vfs = Vfs::new();
    let home_layer = vfs
        .add_home_data_dir(engine_options)
        .map_err(|e| e.to_string())?
        .map(|_| 0);
    for mod_id in engine_options.mods.iter().rev() {
        match mod_manager.get_mod_by_id(mod_id) {
            Some(m) => vfs.add_mod_data_dir(m.path()).map_err(|e| e.to_string())?,
            None => warn!(
                "Mod {} not found, ignoring it for the game settings",
                mod_id
            ),
        }
    }
    vfs.add_externalized_dir(engine_options)
        .map_err(|e| e.to_string())?;

    let skipped: Vec<usize> = home_layer.into_iter().collect();
    let value = vfs
        .read_patched_json_skipping_patches_in(&Nfc::caseless_path(GAME_JSON), &skipped)
        .map_err(|e| format!("Could not read {}: {}", GAME_JSON, e))?;
    if !value.is_object() {
        return Err(format!("{} is not an object", GAME_JSON));
    }
    Ok(value)
}

/// Whether a patch operation that sets a value would succeed on json
fn would_apply(json: &Value, op: &str, pointer: &str) -> bool {
    if op == "replace" {
        return json.pointer(pointer).is_some();
    }
    // add needs the parent to exist
    match pointer.rfind('/') {
        Some(pos) => match json.pointer(&pointer[..pos]) {
            Some(Value::Object(_)) => true,
            Some(Value::Array(a)) => {
                let token = &pointer[pos + 1..];
                token == "-" || token.parse::<usize>().is_ok_and(|i| i <= a.len())
            }
            _ => false,
        },
        None => pointer.is_empty(),
    }
}

// The order of the properties is lost when the schema is compiled into the library,
// so follow game.json
fn build_settings(game_json: &Value, schema: &Value) -> (Vec<Option<String>>, Vec<GameSetting>) {
    let mut sections = Vec::new();
    let mut settings = Vec::new();
    let Some(game_json) = game_json.as_object() else {
        return (sections, settings);
    };
    // Index of the current section for top level settings
    let mut top_level_section = None;
    for (key, value) in game_json {
        let Some(property_schema) = property_schema(schema, key) else {
            continue;
        };
        if let Value::Object(object) = value {
            top_level_section = None;
            let section = sections.len();
            let count = settings.len();
            collect_settings(
                &[key.as_str()],
                None,
                object,
                property_schema,
                section,
                &mut settings,
            );
            if settings.len() > count {
                sections.push(Some(title_of(key, property_schema)));
            }
        } else {
            let section = *top_level_section.get_or_insert(sections.len());
            if let Some(setting) =
                create_setting(&[key.as_str()], None, value, property_schema, section)
            {
                if section == sections.len() {
                    sections.push(None);
                }
                settings.push(setting);
            }
        }
    }
    (sections, settings)
}

// Objects below the section level are shown as a title prefix of their settings
fn collect_settings(
    path: &[&str],
    title_prefix: Option<&str>,
    json: &Map<String, Value>,
    schema: &Value,
    section: usize,
    settings: &mut Vec<GameSetting>,
) {
    for (key, value) in json {
        let Some(property_schema) = property_schema(schema, key) else {
            continue;
        };
        let mut child_path = path.to_vec();
        child_path.push(key);
        if let Value::Object(object) = value {
            let title = title_of(key, property_schema);
            let prefix = match title_prefix {
                Some(p) => format!("{} / {}", p, title),
                None => title,
            };
            collect_settings(
                &child_path,
                Some(&prefix),
                object,
                property_schema,
                section,
                settings,
            );
        } else if let Some(setting) =
            create_setting(&child_path, title_prefix, value, property_schema, section)
        {
            settings.push(setting);
        }
    }
}

// Arrays, nulls and values that do not match the type in the schema are not supported,
// they are left untouched
fn create_setting(
    path: &[&str],
    title_prefix: Option<&str>,
    value: &Value,
    schema: &Value,
    section: usize,
) -> Option<GameSetting> {
    let is_integer = value.is_i64() || value.is_u64();
    let setting_type = match schema.get("type").and_then(Value::as_str)? {
        "boolean" if value.is_boolean() => GameSettingType::Boolean,
        "integer" if is_integer => GameSettingType::Integer,
        // The game reads some numbers as integers, keep them integers
        "number" if is_integer => GameSettingType::Integer,
        "number" if value.is_number() => GameSettingType::Decimal,
        "string" if value.is_string() => GameSettingType::Text,
        _ => return None,
    };
    let key = path.last()?;
    let title = title_of(key, schema);
    let validator = match Validator::options().with_draft(Draft::Draft4).build(schema) {
        Ok(v) => Some(v),
        Err(e) => {
            warn!("Could not compile the schema of {}: {}", key, e);
            None
        }
    };
    let mut pointer = String::new();
    for p in path {
        pointer.push('/');
        pointer.push_str(&p.replace('~', "~0").replace('/', "~1"));
    }
    Some(GameSetting {
        pointer,
        section,
        title: match title_prefix {
            Some(p) => format!("{} / {}", p, title),
            None => title,
        },
        description: string_of(schema, "description").and_then(description_without_type),
        allowed_values: allowed_values_text(schema),
        setting_type,
        default_value: value.clone(),
        validator,
    })
}

fn property_schema<'a>(schema: &'a Value, key: &str) -> Option<&'a Value> {
    schema.get("properties")?.get(key).filter(|s| s.is_object())
}

fn string_of<'a>(schema: &'a Value, name: &str) -> Option<&'a str> {
    schema.get(name).and_then(Value::as_str)
}

fn title_of(key: &str, schema: &Value) -> String {
    if let Some(title) = string_of(schema, "title") {
        return title.to_owned();
    }
    let title = key.replace('_', " ");
    let mut chars = title.chars();
    match chars.next() {
        Some(first) => first.to_uppercase().chain(chars).collect(),
        None => title,
    }
}

// The schema build appends "Type: <title of the referenced type>" to descriptions (see build.rs),
// which is meant for modders, not players
fn description_without_type(description: &str) -> Option<String> {
    let mut description = description.trim_end();
    let type_line = description
        .rfind("\nType: ")
        .filter(|&pos| !description[pos + 1..].contains('\n'));
    if let Some(pos) = type_line {
        description = &description[..pos];
    }
    let description = description.trim();
    (!description.is_empty()).then(|| description.to_owned())
}

// Ranges that only reflect the storage type (e.g. int32) are not meaningful to players and are omitted
fn allowed_values_text(schema: &Value) -> Option<String> {
    const STORAGE_TYPE_RANGES: [(f64, f64); 6] = [
        (i8::MIN as f64, i8::MAX as f64),
        (i16::MIN as f64, i16::MAX as f64),
        (i32::MIN as f64, i32::MAX as f64),
        (0.0, u8::MAX as f64),
        (0.0, u16::MAX as f64),
        (0.0, u32::MAX as f64),
    ];
    let minimum = schema.get("minimum").and_then(Value::as_f64);
    let maximum = schema.get("maximum").and_then(Value::as_f64);
    match (minimum, maximum) {
        (Some(min), Some(max)) if STORAGE_TYPE_RANGES.contains(&(min, max)) => None,
        (Some(min), Some(max)) => Some(format!("{} – {}", format_limit(min), format_limit(max))),
        (Some(min), None) => Some(format!("≥ {}", format_limit(min))),
        (None, Some(max)) => Some(format!("≤ {}", format_limit(max))),
        (None, None) => None,
    }
}

fn format_limit(limit: f64) -> String {
    if limit.fract() == 0.0 {
        (limit as i64).to_string()
    } else {
        limit.to_string()
    }
}
