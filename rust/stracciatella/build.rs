use serde_json::Value;
use std::collections::HashMap;
use std::env;
use std::fs;

const SCHEMA_DIR: &str = "src/schemas/yaml";

fn main() {
    println!("cargo:rerun-if-changed=src/schemas/yaml");

    let mut schemas: HashMap<String, Value> = Default::default();

    for entry in fs::read_dir(SCHEMA_DIR).expect("schema dir is not readable") {
        let entry = entry.expect("entry not readable");

        if entry.file_type().expect("file type").is_dir() {
            continue;
        }
        let file_name = entry
            .file_name()
            .to_str()
            .expect("filename string conversion failed")
            .to_lowercase();
        if !file_name.ends_with(".schema.yml") && !file_name.ends_with(".schema.yaml") {
            continue;
        }

        let schema = fs::read(entry.path()).expect("schema not readable");
        let mut schema: Value = serde_yaml::from_slice(&schema).expect("schema is not valid yaml");
        resolve_refs(&mut schema);

        let file_name = file_name
            .replace(".schema.yml", "")
            .replace(".schema.yaml", "");
        schemas.insert(file_name, schema);
    }

    let schemas_json = serde_json::to_string(&schemas).expect("schemas serialization");

    let out_dir = env::var_os("OUT_DIR").expect("OUT_DIR environment variable not set");
    let out_file = format!(
        "{}/schemas.json",
        out_dir
            .to_str()
            .expect("OUT_DIR variable could not be converted into str")
    );

    fs::write(&out_file, schemas_json).expect("failed to write schemas json");

    println!("cargo:rustc-env=STRACCIATELLA_SCHEMAS={}", out_file)
}

fn resolve_refs(value: &mut Value) {
    if let Some(array) = value.as_array_mut() {
        for item in array.iter_mut() {
            resolve_refs(item);
        }
    }
    if let Some(obj) = value.as_object_mut() {
        if let Some(link) = obj.get("$ref") {
            // Resolve file ref link (currently reflinks to definitions are not supported)
            let original_title = obj.get("title");
            let original_description = obj.get("description").and_then(|v| v.as_str());
            let link = link.as_str().expect("ref is not a string");
            let link = format!("{}/{}", SCHEMA_DIR, link);
            let schema = fs::read(&link).expect("ref schema not readable");
            let mut schema: Value =
                serde_yaml::from_slice(&schema).expect("ref schema is not valid yaml");
            resolve_refs(&mut schema);
            let mut schema = schema
                .as_object()
                .expect("ref schema should be an object")
                .clone();

            if let Some(original_description) = original_description {
                let type_title = schema.get("title").and_then(|v| v.as_str());
                if let Some(type_description) = type_title {
                    let description: Value =
                        format!("{}\n\nType: {}", original_description, type_description).into();
                    schema.insert("description".to_owned(), description);
                } else {
                    schema.insert("description".to_owned(), original_description.into());
                }
            }
            // Merge description and title for the final schema
            if let Some(original_title) = original_title {
                schema.insert("title".to_owned(), original_title.clone());
            } else {
                schema.remove("title");
            }

            *obj = schema
        } else {
            for (_, value) in obj.iter_mut() {
                resolve_refs(value)
            }
        }
    }
}
