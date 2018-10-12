extern crate cbindgen;

use std::path;
use std::env;
use std::fs;

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let header_env = env::var("HEADER_LOCATION").unwrap();
    let header_path = path::PathBuf::from(&header_env);
    let header_dir = header_path.parent().unwrap();
    let config = cbindgen::Config::from_file("cbindgen.toml").expect("Failed to read `cbindgen.toml`!");

    fs::create_dir_all(&header_dir).unwrap();

    cbindgen::Builder::new()
      .with_crate(crate_dir)
      .with_config(config)
      .generate()
      .expect("Unable to generate bindings")
      .write_to_file(&header_path);
}