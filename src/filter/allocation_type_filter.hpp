#pragma once

#include <set>
#include <string>

#include "filter.hpp"

class allocation_type_filter : filter
{
public:
	allocation_type_filter(std::set<std::string> filter);

	virtual bool sampledObjectAlloc(jvmtiEnv *jvmti_env,
									JNIEnv *jni_env,
									jthread thread,
									jobject object,
									jclass object_klass,
									jlong size);

private:
	std::set<std::string> _filter;
};