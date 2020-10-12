use std::path::{Path, PathBuf};
use std::sync::RwLock;

use jni::{
    errors::{ErrorKind, Result},
    objects::{JObject, JString, JValue},
    JNIEnv,
};
use lazy_static::lazy_static;
use ndk::asset::AssetManager;

struct GlobalJNIEnv(pub *mut jni::sys::JNIEnv);

unsafe impl Sync for GlobalJNIEnv {}

unsafe impl Send for GlobalJNIEnv {}

lazy_static! {
    /// This is the global jni env
    /// When the engine is powered up, we expect this to be set once by calling set_global_jni_env.
    ///
    /// We expect the passed pointer to be valid for all subsequent calls that use jni.
    static ref GLOBAL_JNI_ENV: RwLock<GlobalJNIEnv> = {
        RwLock::new(GlobalJNIEnv(std::ptr::null_mut()))
    };
}

/// Set the global jni env for the stracciatella lib.
///
/// This should be called with the SDL JNI env
pub fn set_global_jni_env(jni_env: *mut jni::sys::JNIEnv) -> Result<()> {
    if jni_env.is_null() {
        return Err(ErrorKind::NullPtr("jni_env").into());
    }
    let mut global_jni_env = GLOBAL_JNI_ENV.try_write().map_err(|_| ErrorKind::TryLock)?;
    global_jni_env.0 = jni_env;
    Ok(())
}

/// Gets the rust version of the current global jni env
pub fn get_global_jni_env() -> Result<jni::JNIEnv<'static>> {
    let global_jni_env = GLOBAL_JNI_ENV.try_read().map_err(|_| ErrorKind::TryLock)?;
    unsafe { JNIEnv::from_raw(global_jni_env.0) }
}

/// Get current application context form JNI env
pub fn get_application_context() -> Result<JObject<'static>> {
    let jni_env = get_global_jni_env()?;
    let current_activity_thread = jni_env
        .call_static_method(
            "android/app/ActivityThread",
            "currentActivityThread",
            "()Landroid/app/ActivityThread;",
            &[],
        )?
        .l()?;
    let application = jni_env
        .call_method(
            current_activity_thread,
            "getApplication",
            "()Landroid/app/Application;",
            &[],
        )?
        .l()?;
    jni_env
        .call_method(
            application,
            "getApplicationContext",
            "()Landroid/content/Context;",
            &[],
        )?
        .l()
}

/// Find ja2 stracciatella configuration directory for android
pub fn get_android_app_dir() -> Result<PathBuf> {
    let jni_env = crate::android::get_global_jni_env()?;
    let context = crate::android::get_application_context()?;
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

/// Get asset manager from JNI env
pub fn get_asset_manager() -> Result<AssetManager> {
    let jni_env = get_global_jni_env()?;
    let context = get_application_context()?;
    let asset_manager = jni_env
        .call_method(
            context,
            "getAssets",
            "()Landroid/content/res/AssetManager;",
            &[],
        )?
        .l()?;
    Ok(unsafe {
        // This is the cast of death
        let ptr = ndk_sys::AAssetManager_fromJava(
            (jni_env.get_native_interface() as *const ndk_sys::JNINativeInterface)
                as *mut *const ndk_sys::JNINativeInterface,
            asset_manager.into_inner() as *mut std::ffi::c_void,
        );
        AssetManager::from_ptr(std::ptr::NonNull::new(ptr).unwrap())
    })
}

/// List directory in Assets (including directories)
pub fn list_asset_dir(dir: &Path) -> Result<Vec<PathBuf>> {
    let jni_env = get_global_jni_env()?;
    let context = get_application_context()?;
    let asset_manager = jni_env
        .call_method(
            context,
            "getAssets",
            "()Landroid/content/res/AssetManager;",
            &[],
        )?
        .l()?;
    let path = dir.to_str().map(String::from).ok_or_else(|| {
        ErrorKind::Msg("Error casting path to string for list_asset_dir".to_owned())
    })?;
    let path = JValue::Object(jni_env.new_string(&path)?.into());
    let list_contents = jni_env
        .call_method(
            asset_manager,
            "list",
            "(Ljava/lang/String;)[Ljava/lang/String;",
            &[path],
        )?
        .l()?
        .into_inner();

    let n_elements = jni_env.get_array_length(list_contents)?;
    let mut results: Vec<PathBuf> = vec![];
    for l in 0..n_elements {
        let l: String = jni_env
            .get_string(JString::from(
                jni_env.get_object_array_element(list_contents, l)?,
            ))?
            .into();
        results.push(PathBuf::from(&l));
    }

    Ok(results)
}
