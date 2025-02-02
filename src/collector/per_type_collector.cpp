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
	const auto error = jvmti_env->GetClassSignature(object_klass, &className, &genericPart);

	std::lock_guard lock(this->_m);
	_allocations[error ? "unknown" : className] += size;
}

extern "C" JNIEXPORT jlong JNICALL Java_dk_stuart_jtestmemprofiler_NativePerTypeCollector_init(JNIEnv *env, jclass klass)
{
	return reinterpret_cast<jlong>(new per_type_collector());
}

extern "C" JNIEXPORT void JNICALL Java_dk_stuart_jtestmemprofiler_NativePerTypeCollector_cleanup(JNIEnv *env, jclass klass, jlong nativeHandle)
{
	delete reinterpret_cast<per_type_collector *>(nativeHandle);
}

extern "C" JNIEXPORT jobject JNICALL Java_dk_stuart_jtestmemprofiler_NativePerTypeCollector_get(JNIEnv *env, jclass klass, jlong nativeHandle)
{
	const auto collector = reinterpret_cast<per_type_collector *>(nativeHandle);

	const auto hashMapClass = env->FindClass("java/util/HashMap");
	const auto hashMapConstructor = env->GetMethodID(hashMapClass, "<init>", "()V");
	const auto putMethod = env->GetMethodID(hashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

	const auto longClass = env->FindClass("java/lang/Long");
	const auto longConstructor = env->GetMethodID(longClass, "<init>", "(J)V");

	const auto hashMap = env->NewObject(hashMapClass, hashMapConstructor);

	for (const auto &[fst, snd] : collector->getAllocations())
	{
		const auto key = env->NewStringUTF(fst.c_str());
		const auto value = env->NewObject(longClass, longConstructor, snd);

		env->CallObjectMethod(hashMap, putMethod, key, value);

		env->DeleteLocalRef(key);
		env->DeleteLocalRef(value);
	}

	env->DeleteLocalRef(hashMapClass);
	env->DeleteLocalRef(longClass);

	return hashMap;
}
