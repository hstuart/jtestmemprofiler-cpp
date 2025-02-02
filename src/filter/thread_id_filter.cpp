#include "thread_id_filter.hpp"

#include <utility>

thread_id_filter::thread_id_filter(std::set<jlong> filter) : _filter(std::move(filter)) {}

bool thread_id_filter::sampledObjectAlloc(jvmtiEnv *jvmti_env, JNIEnv *jni_env, jthread thread, jobject object, jclass object_klass, jlong size)
{
    const auto threadCls = jni_env->GetObjectClass(thread);
    const auto methodId = jni_env->GetMethodID(threadCls, "getId", "()J");
    const auto threadId = jni_env->CallLongMethod(thread, methodId);

    const auto found = _filter.contains(threadId);

    jni_env->DeleteLocalRef(threadCls);

    return found;
}

extern "C" JNIEXPORT jlong JNICALL Java_dk_stuart_jtestmemprofiler_NativeThreadIdFilter_init(JNIEnv *env, jclass klass, jobject setOfThreads)
{
    const auto setClass = env->GetObjectClass(setOfThreads);
    const auto iteratorMethod = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
    const auto iterator = env->CallObjectMethod(setOfThreads, iteratorMethod);

    const auto iteratorClass = env->GetObjectClass(iterator);
    const auto hasNextMethod = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    const auto nextMethod = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");

    std::set<jlong> filter;

    while (env->CallBooleanMethod(iterator, hasNextMethod))
    {
        const auto classObj = env->CallObjectMethod(iterator, nextMethod);
        const auto classClass = env->GetObjectClass(classObj);
        const auto getIdMethod = env->GetMethodID(classClass, "getId", "()J");
        const auto threadId = env->CallLongMethod(classObj, getIdMethod);

        filter.insert(threadId);

        env->DeleteLocalRef(classObj);
        env->DeleteLocalRef(classClass);
    }

    // Clean up local references
    env->DeleteLocalRef(iterator);
    env->DeleteLocalRef(setClass);

    return reinterpret_cast<jlong>(new thread_id_filter(filter));
}

extern "C" JNIEXPORT void JNICALL Java_dk_stuart_jtestmemprofiler_NativeThreadIdFilter_cleanup(JNIEnv *env, jclass klass, const jlong nativeHandle)
{
    delete reinterpret_cast<thread_id_filter *>(nativeHandle);
}
