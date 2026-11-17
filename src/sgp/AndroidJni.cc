// Native functions called from the Android launcher.
//
// They are defined here rather than in the rust library because the rust library
// is linked statically, and the linker would drop JNI functions that nothing else references.

#ifdef __ANDROID__

#include "stracciatella.h"

#include <jni.h>

#include <cstring>

// Returns the json schema for a path in externalized dir as UTF-8, or null if there is none.
// The schema is returned as bytes because NewStringUTF expects modified UTF-8.
extern "C" JNIEXPORT jbyteArray JNICALL
Java_io_github_ja2stracciatella_NativeSchemas_getSchemaBytesForPath(JNIEnv* env, jobject, jstring path)
{
	static RustPointer<SchemaManager> schemaManager(SchemaManager_create());

	const char* pathChars = env->GetStringUTFChars(path, nullptr);
	if (pathChars == nullptr) {
		// OutOfMemoryError has been thrown
		return nullptr;
	}
	RustPointer<char> schema(SchemaManager_getSchemaForPath(schemaManager.get(), pathChars));
	env->ReleaseStringUTFChars(path, pathChars);

	if (schema == nullptr) {
		return nullptr;
	}
	jsize length = static_cast<jsize>(std::strlen(schema.get()));
	jbyteArray bytes = env->NewByteArray(length);
	if (bytes == nullptr) {
		// OutOfMemoryError has been thrown
		return nullptr;
	}
	env->SetByteArrayRegion(bytes, 0, length, reinterpret_cast<const jbyte*>(schema.get()));
	return bytes;
}

#endif
