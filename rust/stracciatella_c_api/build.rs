use std::env;
use std::path::PathBuf;

/// Generates the `stracciatella.h` header according to `cbindgen.toml`.
/// The header file will only be updated if the generated text is different from the existing text.
fn stracciatella_h() {
    let crate_dir: PathBuf = env::var_os("CARGO_MANIFEST_DIR").unwrap().into();
    let header_path: PathBuf = if let Some(location) = env::var_os("HEADER_LOCATION") {
        location.into() // path from env
    } else {
        crate_dir.join("stracciatella.h") // default path
    };

    cbindgen::generate(&crate_dir)
        .expect("Unable to generate bindings")
        .write_to_file(&header_path);
    println!("cargo:rerun-if-changed={:?}", &header_path);
}

fn main() {
    stracciatella_h();
}
