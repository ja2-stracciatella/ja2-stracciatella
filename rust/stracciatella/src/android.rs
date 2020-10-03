use jni::{errors::Result, objects::JObject, JNIEnv};
use ndk::asset::AssetManager;

/// Get current application context form JNI env
#[cfg(target_os = "android")]
pub fn get_application_context(jni_env: JNIEnv<'_>) -> Result<JObject> {
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

/// Get asset manager from JNI env
#[cfg(target_os = "android")]
pub fn get_asset_manager(jni_env: JNIEnv<'_>) -> Result<AssetManager> {
    let context = get_application_context(jni_env.clone())?;
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
