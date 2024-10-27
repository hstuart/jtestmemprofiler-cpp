#pragma once

#include <jvmti.h>

class filter
{
public:
	virtual bool sampledObjectAlloc(jvmtiEnv *jvmti_env,
									JNIEnv *jni_env,
									jthread thread,
									jobject object,
									jclass object_klass,
									jlong size) = 0;
};