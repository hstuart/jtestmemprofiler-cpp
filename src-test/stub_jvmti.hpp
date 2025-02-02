#pragma once

#include <jvmti.h>

typedef jvmtiError (*GetMethodNameType)(jvmtiEnv *env, jmethodID method, char **name_ptr, char **signature_ptr,
                                        char **generic_ptr);

class StubJvmtiFunctions : public jvmtiInterface_1 {
public:
    StubJvmtiFunctions() : jvmtiInterface_1() {
    }

    void setGetMethodName(GetMethodNameType getMethodName) {
        GetMethodName = getMethodName;
    }

    void setGetMethodDeclaringClass(
        jvmtiError getMethodDeclaringClass(jvmtiEnv *env, jmethodID method, jclass *declaring_class_ptr)) {
        GetMethodDeclaringClass = getMethodDeclaringClass;
    }

    void setGetClassSignature(
        jvmtiError getClassSignature(jvmtiEnv *env, jclass klass, char **signature_ptr, char **generic_ptr)) {
        GetClassSignature = getClassSignature;
    }

    void setDeallocate(jvmtiError deallocate(jvmtiEnv *env, unsigned char *mem)) {
        Deallocate = deallocate;
    }
};

class StubJvmtiEnv : public _jvmtiEnv {
public:
    explicit StubJvmtiEnv(StubJvmtiFunctions *functions) : _jvmtiEnv() {
        this->functions = functions;
    }
};

inline std::vector<jvmtiFrameInfo> create_frame_infos(int offset, int count) {
    std::vector<jvmtiFrameInfo> frames(count);
    for (int i = 0; i < count; ++i) {
        frames[i].method = reinterpret_cast<jmethodID>(static_cast<intptr_t>(i + 1 + offset));
    }
    return frames;
}
