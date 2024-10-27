#include "allocation_type_filter.hpp"

allocation_type_filter::allocation_type_filter(std::set<std::string> filter) : _filter(filter) {}

bool allocation_type_filter::sampledObjectAlloc(jvmtiEnv *jvmti_env, JNIEnv *jni_env, jthread thread, jobject object, jclass object_klass, jlong size)
{
    jclass classClass = jni_env->FindClass("java/lang/Class");
    auto getNameMethod = jni_env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
    auto className = (jstring)jni_env->CallObjectMethod(object_klass, getNameMethod);

    const char *classNameCStr = jni_env->GetStringUTFChars(className, NULL);

    auto found = _filter.find(classNameCStr) != _filter.end();

    jni_env->ReleaseStringUTFChars(className, classNameCStr);
    jni_env->DeleteLocalRef(className);
    jni_env->DeleteLocalRef(classClass);

    return found;
}

extern "C" JNIEXPORT jlong JNICALL Java_dk_stuart_jtestmemprofiler_NativeAllocationTypeFilter_init(JNIEnv *env, jclass klass, jobject setOfKlasses)
{
    auto setClass = env->GetObjectClass(setOfKlasses);
    auto iteratorMethod = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
    auto iterator = env->CallObjectMethod(setOfKlasses, iteratorMethod);

    auto iteratorClass = env->GetObjectClass(iterator);
    auto hasNextMethod = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    auto nextMethod = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");

    std::set<std::string> filter;

    while (env->CallBooleanMethod(iterator, hasNextMethod))
    {
        auto classObj = env->CallObjectMethod(iterator, nextMethod);
        auto classClass = env->GetObjectClass(classObj);
        auto getNameMethod = env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
        auto className = (jstring)env->CallObjectMethod(classObj, getNameMethod);

        const char *classNameCStr = env->GetStringUTFChars(className, NULL);
        filter.insert(std::string(classNameCStr));

        env->ReleaseStringUTFChars(className, classNameCStr);
        env->DeleteLocalRef(classObj);
        env->DeleteLocalRef(classClass);
        env->DeleteLocalRef(className);
    }

    env->DeleteLocalRef(iterator);
    env->DeleteLocalRef(setClass);

    return (jlong) new allocation_type_filter(filter);
}

extern "C" JNIEXPORT void JNICALL Java_dk_stuart_jtestmemprofiler_NativeAllocationTypeFilter_cleanup(JNIEnv *env, jclass klass, jlong nativeHandle)
{
    if (nullptr != (allocation_type_filter *)nativeHandle)
        delete (allocation_type_filter *)nativeHandle;
}
