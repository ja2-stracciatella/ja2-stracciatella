#[cfg(target_os = "android")]
use jni::{errors::Error as JNIError, JNIEnv};
use std::path::PathBuf;

/// Find ja2 stracciatella configuration directory inside the user's home directory
#[cfg(not(target_os = "android"))]
pub fn find_stracciatella_home() -> Result<PathBuf, String> {
    use crate::fs::resolve_existing_components;

    #[cfg(not(windows))]
    let base = dirs::home_dir();
    #[cfg(windows)]
    let base = dirs::document_dir();
    #[cfg(not(windows))]
    let dir = ".ja2";
    #[cfg(windows)]
    let dir = "JA2";

    match base {
        Some(mut path) => {
            path.push(dir);
            Ok(resolve_existing_components(&path, None, true))
        }
        None => Err(String::from("Could not find home directory")),
    }
}

/// Find ja2 stracciatella configuration directory for android
/// This is a separate function as it needs the jnienv argument
#[cfg(target_os = "android")]
pub fn find_stracciatella_home(jni_env: JNIEnv<'_>) -> Result<PathBuf, JNIError> {
    let context = crate::android::get_application_context(jni_env.clone())?;
    let files_dir = jni_env
        .call_method(context, "getFilesDir", "()Ljava/io/File;", &[])?
        .l()?;
    let path = jni_env.get_string(
        jni_env
            .call_method(files_dir, "getAbsolutePath", "()Ljava/lang/String;", &[])?
            .l()?
            .into(),
    )?;
    let path_string: String = path.into();

    Ok(PathBuf::from(&path_string))
}
