#pragma once

#include <jvmti.h>

class collector
{
public:
	virtual ~collector() = default;

	virtual void sampledObjectAlloc(jvmtiEnv *jvmti_env,
	                                JNIEnv *jni_env,
	                                jthread thread,
	                                jobject object,
	                                jclass object_klass,
	                                jlong size) = 0;
};
