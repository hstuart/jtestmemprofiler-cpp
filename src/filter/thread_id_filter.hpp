#pragma once

#include <set>

#include "filter.hpp"

class thread_id_filter : filter
{
public:
	thread_id_filter(std::set<jlong> filter);

	virtual bool sampledObjectAlloc(jvmtiEnv *jvmti_env,
									JNIEnv *jni_env,
									jthread thread,
									jobject object,
									jclass object_klass,
									jlong size);

private:
	std::set<jlong> _filter;
};
