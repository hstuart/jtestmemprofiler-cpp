#include "per_type_collector.hpp"

const std::map<std::string, jlong> &per_type_collector::getAllocations()
{
	return _allocations;
}

void per_type_collector::sampledObjectAlloc(jvmtiEnv *jvmti_env,
											JNIEnv *jni_env,
											jthread thread,
											jobject object,
											jclass object_klass,
											jlong size)
{

	char *className = nullptr;
	char *genericPart = nullptr;
	auto error = jvmti_env->GetClassSignature(object_klass, &className, &genericPart);

	std::lock_guard<std::mutex> lock(this->_m);
	_allocations[error ? "unknown" : className] += size;
}

extern "C" JNIEXPORT jlong JNICALL Java_dk_stuart_jtestmemprofiler_NativePerTypeCollector_init(JNIEnv *env, jclass klass)
{
	return (jlong) new per_type_collector();
}

extern "C" JNIEXPORT void JNICALL Java_dk_stuart_jtestmemprofiler_NativePerTypeCollector_cleanup(JNIEnv *env, jclass klass, jlong nativeHandle)
{
	if (nullptr != (per_type_collector *)nativeHandle)
		delete (per_type_collector *)nativeHandle;
}

extern "C" JNIEXPORT jobject JNICALL Java_dk_stuart_jtestmemprofiler_NativePerTypeCollector_get(JNIEnv *env, jclass klass, jlong nativeHandle)
{
	auto collector = (per_type_collector *)nativeHandle;

	auto hashMapClass = env->FindClass("java/util/HashMap");
	auto hashMapConstructor = env->GetMethodID(hashMapClass, "<init>", "()V");
	auto putMethod = env->GetMethodID(hashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

	auto longClass = env->FindClass("java/lang/Long");
	auto longConstructor = env->GetMethodID(longClass, "<init>", "(J)V");

	auto hashMap = env->NewObject(hashMapClass, hashMapConstructor);

	for (const auto &entry : collector->getAllocations())
	{
		auto key = env->NewStringUTF(entry.first.c_str());
		auto value = env->NewObject(longClass, longConstructor, entry.second);

		env->CallObjectMethod(hashMap, putMethod, key, value);

		env->DeleteLocalRef(key);
		env->DeleteLocalRef(value);
	}

	env->DeleteLocalRef(hashMapClass);
	env->DeleteLocalRef(longClass);

	return hashMap;
}
