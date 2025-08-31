//! Json schema handling for resource files
//!
//! Json schemas are written as YAML into the `yaml/` subfolder of this module.
//! The build script will resolve references within the schema and put it into a file
//! This file path is set as`STRACCIATELLA_SCHEMAS` env variable.
//!
//! `STRACCIATELLA_SCHEMAS` is read at build time and can be accessed using `SchemaManager`

use std::{collections::HashMap, convert::TryFrom, error::Error, path::Path};

use jsonschema::{Draft, JSONSchema};
use serde_json::Value;

// This is set by the build script and contains all schemas in resolved json form
const SCHEMAS: &str = include_str!(env!("STRACCIATELLA_SCHEMAS"));

/// A representation of a json schema
pub struct Schema {
    compiled: JSONSchema,
    inner_value: Value,
    inner_str: String,
}

impl Schema {
    /// Get string representation of the json schema
    pub fn as_str(&self) -> &str {
        &self.inner_str
    }

    /// Get json value of the json schema
    pub fn as_value(&self) -> &Value {
        &self.inner_value
    }
}

impl TryFrom<Value> for Schema {
    type Error = Box<dyn Error>;

    fn try_from(value: Value) -> Result<Self, Self::Error> {
        let compiled = JSONSchema::options()
            .with_draft(Draft::Draft4)
            .compile(&value)
            .map_err(|e| e.to_string())?;
        let inner_str = serde_json::to_string(&value)?;
        Ok(Self {
            compiled,
            inner_value: value,
            inner_str,
        })
    }
}

/// The schema manager contains all available json schemas for resource files
pub struct SchemaManager {
    /// Hashmap of strings to schemas as serialized JSON
    schemas: HashMap<String, Schema>,
}

impl SchemaManager {
    /// Gets json schema for a specific resource path.
    ///
    /// **Note**: Path is not the path to the schema, but to the resource file itself.
    pub fn get(&self, path: &Path) -> Option<&Schema> {
        let path = path.to_string_lossy().to_lowercase().replace(".json", "");
        if path.starts_with("dealer-inventory-") || path.starts_with("bobby-ray-inventory-") {
            return self.schemas.get("dealer-inventory");
        }
        if path.starts_with("army-gun-choice-") {
            return self.schemas.get("army-gun-choice");
        }
        if path.starts_with("script-records-control") {
            return self.schemas.get("script-records-control");
        } else if path.starts_with("script-records-") {
            return self.schemas.get("script-records");
        }
        if path.starts_with("translation_tables/") {
            return self.schemas.get("translation-table");
        }
        if path.starts_with("strings/") {
            return self.schemas.get("strings");
        }
        self.schemas.get(&path)
    }

    // Validates a value loaded from path and returns validation errors
    pub fn validate(&self, path: &Path, value: &Value) -> Option<Vec<String>> {
        if let Some(schema) = self.get(path) {
            schema.compiled.validate(value).err().map(|e| {
                e.map(|v| format!("Error at JSON path `{}`: {}", v.instance_path, v))
                    .collect()
            })
        } else {
            Some(vec![format!(
                "could not find schema for path `{}`",
                path.to_string_lossy()
            )])
        }
    }

    /// Get all json schemas
    pub fn get_all(&self) -> &HashMap<String, Schema> {
        &self.schemas
    }
}

impl Default for SchemaManager {
    fn default() -> Self {
        let schemas: HashMap<String, Value> = serde_json::from_str(SCHEMAS)
            .expect("STRACCIATELLA_SCHEMAS set by build script, should be valid json");
        let schemas: HashMap<String, Schema> = schemas
            .into_iter()
            .map(|(key, schema)| {
                let schema = Schema::try_from(schema)
                    .expect("STRACCIATELLA_SCHEMAS set by build script, should be serializable");
                (key, schema)
            })
            .collect();
        Self { schemas }
    }
}

#[cfg(test)]
mod tests {
    use std::path::Path;

    use super::SchemaManager;

    #[test]
    fn it_should_contain_schemas_for_specific_paths() {
        let schema_manager = SchemaManager::default();
        let paths = vec![
            "dealer-inventory-alberto-santos.json",
            "ammo-types.json",
            "strings/test.json",
            "game.json",
        ];
        for path_str in paths {
            let path = Path::new(path_str);
            assert!(schema_manager.get(path).is_some(), "{}", path_str);
        }
    }
}
