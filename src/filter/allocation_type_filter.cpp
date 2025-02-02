#include "allocation_type_filter.hpp"

#include <utility>

allocation_type_filter::allocation_type_filter(std::set<std::string> filter) : _filter(std::move(filter)) {}

bool allocation_type_filter::sampledObjectAlloc(jvmtiEnv *jvmti_env, JNIEnv *jni_env, jthread thread, jobject object, jclass object_klass, jlong size)
{
    const auto classClass = jni_env->FindClass("java/lang/Class");
    const auto getNameMethod = jni_env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
    const auto className = reinterpret_cast<jstring>(jni_env->CallObjectMethod(object_klass, getNameMethod));

    const char *classNameCStr = jni_env->GetStringUTFChars(className, nullptr);

    const auto found = _filter.contains(classNameCStr);

    jni_env->ReleaseStringUTFChars(className, classNameCStr);
    jni_env->DeleteLocalRef(className);
    jni_env->DeleteLocalRef(classClass);

    return found;
}

extern "C" JNIEXPORT jlong JNICALL Java_dk_stuart_jtestmemprofiler_NativeAllocationTypeFilter_init(JNIEnv *env, jclass klass, jobject setOfKlasses)
{
    const auto setClass = env->GetObjectClass(setOfKlasses);
    const auto iteratorMethod = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
    const auto iterator = env->CallObjectMethod(setOfKlasses, iteratorMethod);

    const auto iteratorClass = env->GetObjectClass(iterator);
    const auto hasNextMethod = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    const auto nextMethod = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");

    std::set<std::string> filter;

    while (env->CallBooleanMethod(iterator, hasNextMethod))
    {
        const auto classObj = env->CallObjectMethod(iterator, nextMethod);
        const auto classClass = env->GetObjectClass(classObj);
        const auto getNameMethod = env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
        const auto className = reinterpret_cast<jstring>(env->CallObjectMethod(classObj, getNameMethod));

        const char *classNameCStr = env->GetStringUTFChars(className, nullptr);
        filter.insert(std::string(classNameCStr));

        env->ReleaseStringUTFChars(className, classNameCStr);
        env->DeleteLocalRef(classObj);
        env->DeleteLocalRef(classClass);
        env->DeleteLocalRef(className);
    }

    env->DeleteLocalRef(iterator);
    env->DeleteLocalRef(setClass);

    return reinterpret_cast<jlong>(new allocation_type_filter(filter));
}

extern "C" JNIEXPORT void JNICALL Java_dk_stuart_jtestmemprofiler_NativeAllocationTypeFilter_cleanup(JNIEnv *env, jclass klass, jlong nativeHandle)
{
    delete reinterpret_cast<allocation_type_filter *>(nativeHandle);
}
