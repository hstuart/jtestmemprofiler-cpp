#pragma once
#include <jni.h>

struct StubJniFunctions : JNINativeInterface_ {
    StubJniFunctions& setDeleteLocalRef(void deleteLocalRef(JNIEnv *env, jobject object)) {
        DeleteLocalRef = deleteLocalRef;
        return *this;
    }
};

struct StubJniEnv : JNIEnv_ {
    explicit StubJniEnv(StubJniFunctions* functions) : JNIEnv_() {
        this->functions = functions;
    }
};
