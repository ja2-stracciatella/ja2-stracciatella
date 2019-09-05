extern crate cbindgen;

use std::env;
use std::fs;
use std::path;

fn main() {
    let crate_dir = path::PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let default_header_path = crate_dir
        .join("stracciatella.h")
        .into_os_string()
        .into_string()
        .unwrap();
    let header_env = env::var("HEADER_LOCATION").unwrap_or(default_header_path);
    let header_path = path::PathBuf::from(&header_env);
    let header_dir = header_path.parent().unwrap();
    let config =
        cbindgen::Config::from_file("cbindgen.toml").expect("Failed to read `cbindgen.toml`!");

    fs::create_dir_all(&header_dir).unwrap();

    cbindgen::Builder::new()
        .with_crate(crate_dir)
        .with_config(config)
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file(&header_path);
}
