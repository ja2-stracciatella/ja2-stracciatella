//! This module contains miscellaneous code for C.
//!
//! The code in this module is not associated with any module in particular.

use std::env;
use std::ffi::CString;
use std::process::Command;
use std::ptr;

use stracciatella::config::EngineOptions;
use stracciatella::fs::resolve_existing_components;
use stracciatella::get_assets_dir;
use stracciatella::guess::guess_vanilla_version;

use crate::c::common::*;
use crate::c::vec::VecCString;

/// Sets the global JNI env for Android
#[no_mangle]
#[cfg(target_os = "android")]
pub extern "C" fn setGlobalJniEnv(jni_env: *mut jni::sys::JNIEnv) -> bool {
    forget_rust_error();
    if let Err(e) = stracciatella::android::set_global_jni_env(jni_env) {
        remember_rust_error(format!("setGlobalJniEnv: {}", e))
    }
    no_rust_error()
}

/// Deletes a CString.
/// The caller is no longer responsible for the memory.
#[no_mangle]
#[allow(clippy::not_unsafe_ptr_arg_deref)]
pub extern "C" fn CString_destroy(s: *mut c_char) {
    if s.is_null() {
        return;
    }
    unsafe { CString::from_raw(s) };
}

/// Guesses the resource version from the contents of the game directory.
/// Returns a VanillaVersion value if it was sucessful, -1 otherwise.
#[no_mangle]
pub extern "C" fn guessResourceVersion(gamedir: *const c_char) -> c_int {
    let path = str_from_c_str_or_panic(unsafe_c_str(gamedir));
    let logged = guess_vanilla_version(&path);
    let mut result = -1;
    if let Some(version) = logged.vanilla_version {
        result = version as c_int;
    }
    result
}

/// Finds a path relative to the assets directory.
/// If path is null, it returns the assets directory.
/// If test_exists is true and the path does not exist, it returns null.
/// The caller is responsible for the returned memory.
#[no_mangle]
pub extern "C" fn findPathFromAssetsDir(
    path: *const c_char,
    test_exists: bool,
    caseless: bool,
) -> *mut c_char {
    let assets_dir = get_assets_dir();
    let path = if !path.is_null() {
        let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
        resolve_existing_components(&path, Some(&assets_dir), caseless)
    } else {
        assets_dir
    };
    if test_exists && !path.exists() {
        ptr::null_mut()
    } else {
        let c_string = c_string_from_path_or_panic(&path);
        c_string.into_raw()
    }
}

/// Finds a path relative to the stracciatella home directory.
/// If path is null, it finds the stracciatella home directory.
/// If test_exists is true, it makes sure the path exists.
/// The caller is responsible for the returned memory.
#[no_mangle]
pub extern "C" fn findPathFromStracciatellaHome(
    engine_options: *mut EngineOptions,
    path: *const c_char,
    test_exists: bool,
    caseless: bool,
) -> *mut c_char {
    use stracciatella::fs::canonicalize;

    let engine_options = unsafe_mut(engine_options);
    let mut path_buf = engine_options.stracciatella_home.clone();
    if !path.is_null() {
        let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
        let path = resolve_existing_components(&path, Some(&path_buf), caseless);
        path_buf = path;
    }
    if test_exists && !path_buf.exists() {
        ptr::null_mut() // path not found
    } else {
        if let Ok(p) = canonicalize(&path_buf) {
            path_buf = p;
        }
        let s: String = path_buf.to_string_lossy().into();
        CString::new(s).unwrap().into_raw() // path found
    }
}

/// Returns true if it was able to find path relative to base.
/// Makes caseless searches one component at a time.
#[no_mangle]
pub extern "C" fn checkIfRelativePathExists(
    base: *const c_char,
    path: *const c_char,
    caseless: bool,
) -> bool {
    let base = path_buf_from_c_str_or_panic(unsafe_c_str(base));
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    let path = resolve_existing_components(&path, Some(&base), caseless);
    path.exists()
}

/// Returns a list of available mods.
/// The caller is responsible for the returned memory.
#[no_mangle]
pub extern "C" fn findAvailableMods(engine_options: *mut EngineOptions) -> *mut VecCString {
    let engine_options = unsafe_mut(engine_options);
    let mut entries: Vec<_> = Vec::new();

    // list all directories under assets dir
    let mut assets_path = get_assets_dir();
    assets_path.push("mods");
    if let Ok(paths) = assets_path.read_dir() {
        paths.for_each(|p| entries.push(p));
    }

    // list all directories under stracc home dir
    let mut home_dir = engine_options.stracciatella_home.clone();
    home_dir.push("mods");
    if let Ok(paths) = home_dir.read_dir() {
        paths.for_each(|p| entries.push(p));
    }

    let mut mods: Vec<_> = entries
        .into_iter()
        .filter_map(|x| x.ok()) // DirEntry
        .filter_map(|x| {
            if let Ok(metadata) = x.metadata() {
                if metadata.is_dir() {
                    return x.file_name().into_string().ok();
                }
            }
            None
        }) // String
        .filter_map(|x| CString::new(x.as_bytes().to_owned()).ok()) // CString
        .collect();

    // remove duplicates
    mods.sort();
    mods.dedup();

    into_ptr(VecCString::from(mods))
}

/// Executes a command.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Command_execute(program: *const c_char, args: *mut VecCString) -> bool {
    forget_rust_error();
    let program = path_buf_from_c_str_or_panic(unsafe_c_str(program));
    let args: Vec<_> = unsafe_ref(args)
        .inner
        .iter()
        .map(|x| path_buf_from_c_str_or_panic(x))
        .collect();
    let status_result = Command::new(&program).args(&args).status();
    match status_result {
        Err(err) => remember_rust_error(format!(
            "Command_execute {:?} {:?}: failed to execute: {}",
            program, args, err
        )),
        Ok(status) => match status.code() {
            None => remember_rust_error(format!(
                "Command_execute {:?} {:?}: terminated by a signal",
                program, args
            )),
            Some(code) => {
                if code != 0 {
                    remember_rust_error(format!(
                        "Command_execute {:?} {:?}: exited with status code {}",
                        program, args, code
                    ))
                }
            }
        },
    }
    no_rust_error()
}

/// Gets the path to the assets dir.
/// Can be set via EXTRA_DATA_DIR env variable at compilation time
#[no_mangle]
pub extern "C" fn Env_assetsDir() -> *mut c_char {
    c_string_from_path_or_panic(&get_assets_dir()).into_raw()
}

/// Gets the path to the current directory.
/// On error it returns null.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Env_currentDir() -> *mut c_char {
    forget_rust_error();
    match env::current_dir() {
        Ok(path) => {
            let c_path = c_string_from_path_or_panic(&path);
            c_path.into_raw()
        }
        Err(err) => {
            remember_rust_error(format!("Env_currentDir: {}", err));
            ptr::null_mut()
        }
    }
}

/// Sets the path to the current directory.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Env_setCurrentDir(path: *const c_char) -> bool {
    forget_rust_error();
    let path = path_buf_from_c_str_or_panic(unsafe_c_str(path));
    if let Err(err) = env::set_current_dir(&path) {
        remember_rust_error(format!("Env_setCurrentDir {:?}: {}", path, err));
    }
    no_rust_error()
}

/// Gets the path to the current executable.
/// Sets the rust error.
#[no_mangle]
pub extern "C" fn Env_currentExe() -> *mut c_char {
    forget_rust_error();
    match env::current_exe() {
        Err(err) => {
            remember_rust_error(format!("Env_currentExe: {}", err));
            ptr::null_mut()
        }
        Ok(path) => {
            let c_path = c_string_from_path_or_panic(&path);
            c_path.into_raw()
        }
    }
}

#[cfg(test)]
mod tests {
    #![allow(clippy::complexity)]

    use std::ffi::CString;
    use std::fs;

    use tempfile::TempDir;

    #[test]
    fn check_if_relative_path_exists() {
        // The case sensitivity of the filesystem is always unknown.
        // Even parts of the path can have different case sensitivity.
        // Make sure the expected result never depends on the case sensitivity of the filesystem!
        //
        // Different representations of the umlaut ö in utf-8:
        // "utf8-gedöns" can be "utf8-ged\u{00F6}ns" or "utf8-gedo\u{0308}ns"
        // "utf8-GEDÖNS" can be "utf8-GED\u{00D6}NS" or "utf8-GEDO\u{0308}NS"

        let temp_dir = TempDir::new().unwrap();
        fs::create_dir_all(temp_dir.path().join("foo/bar")).unwrap();
        fs::create_dir_all(temp_dir.path().join("with space/inner")).unwrap();
        fs::create_dir_all(temp_dir.path().join("utf8-ged\u{00F6}ns/inner")).unwrap();

        macro_rules! t {
            ($base: expr, $path:expr, $caseless:expr, $expected:expr) => {
                let base = CString::new($base.to_str().unwrap()).unwrap();
                let path = CString::new($path).unwrap();
                assert_eq!(
                    super::checkIfRelativePathExists(base.as_ptr(), path.as_ptr(), $caseless),
                    $expected
                );
            };
        }

        t!(temp_dir.path(), "baz", false, false);
        t!(temp_dir.path(), "baz", true, false);

        t!(temp_dir.path(), "foo", false, true);
        t!(temp_dir.path(), "foo", true, true);
        t!(temp_dir.path(), "FOO", true, true);

        t!(temp_dir.path(), "foo/bar", false, true);
        t!(temp_dir.path(), "foo/bar", true, true);
        t!(temp_dir.path(), "foo/BAR", true, true);
        t!(temp_dir.path(), "FOO/BAR", true, true);
        t!(temp_dir.path(), "FOO/bar", true, true);

        t!(temp_dir.path(), "withspace", false, false);
        t!(temp_dir.path(), "withspace", true, false);
        t!(temp_dir.path(), "with space", false, true);
        t!(temp_dir.path(), "with space", true, true);
        t!(temp_dir.path(), "with SPACE", true, true);

        t!(temp_dir.path(), "with space/inner", false, true);
        t!(temp_dir.path(), "with SPACE/inner", true, true);
        t!(temp_dir.path(), "with SPACE/INNER", true, true);
        t!(temp_dir.path(), "with space/INNER", true, true);

        t!(temp_dir.path(), "utf8-ged\u{00F6}ns/inner", false, true);
        t!(temp_dir.path(), "utf8-ged\u{00F6}ns/inner", true, true);
        t!(temp_dir.path(), "utf8-ged\u{00F6}ns/other", false, false);
        t!(temp_dir.path(), "utf8-gedo\u{0308}ns/inner", true, true);
        t!(temp_dir.path(), "utf8-GED\u{00D6}NS/inner", true, true);
        t!(temp_dir.path(), "utf8-GEDO\u{0308}NS/inner", true, true);
    }
}
