#include "thread_id_filter.hpp"

thread_id_filter::thread_id_filter(std::set<jlong> filter) : _filter(filter) {}

bool thread_id_filter::sampledObjectAlloc(jvmtiEnv *jvmti_env, JNIEnv *jni_env, jthread thread, jobject object, jclass object_klass, jlong size)
{
    auto threadCls = jni_env->GetObjectClass(thread);
    auto methodId = jni_env->GetMethodID(threadCls, "getId", "()J");
    auto threadId = jni_env->CallLongMethod(thread, methodId);

    auto found = _filter.find(threadId) != _filter.end();

    jni_env->DeleteLocalRef(threadCls);

    return found;
}

extern "C" JNIEXPORT jlong JNICALL Java_dk_stuart_jtestmemprofiler_NativeThreadIdFilter_init(JNIEnv *env, jclass klass, jobject setOfThreads)
{
    auto setClass = env->GetObjectClass(setOfThreads);
    auto iteratorMethod = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
    auto iterator = env->CallObjectMethod(setOfThreads, iteratorMethod);

    auto iteratorClass = env->GetObjectClass(iterator);
    auto hasNextMethod = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    auto nextMethod = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");

    std::set<jlong> filter;

    while (env->CallBooleanMethod(iterator, hasNextMethod))
    {
        auto classObj = env->CallObjectMethod(iterator, nextMethod);
        auto classClass = env->GetObjectClass(classObj);
        auto getIdMethod = env->GetMethodID(classClass, "getId", "()J");
        auto threadId = env->CallLongMethod(classObj, getIdMethod);

        filter.insert(threadId);

        env->DeleteLocalRef(classObj);
        env->DeleteLocalRef(classClass);
    }

    // Clean up local references
    env->DeleteLocalRef(iterator);
    env->DeleteLocalRef(setClass);

    return (jlong) new thread_id_filter(filter);
}

extern "C" JNIEXPORT void JNICALL Java_dk_stuart_jtestmemprofiler_NativeThreadIdFilter_cleanup(JNIEnv *env, jclass klass, jlong nativeHandle)
{
    if (nullptr != (thread_id_filter *)nativeHandle)
        delete (thread_id_filter *)nativeHandle;
}
