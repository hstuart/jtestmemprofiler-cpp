#pragma once

#include <set>
#include <string>

#include "filter.hpp"

class allocation_type_filter final : filter
{
public:
	explicit allocation_type_filter(std::set<std::string> filter);

	bool sampledObjectAlloc(jvmtiEnv *jvmti_env,
	                        JNIEnv *jni_env,
	                        jthread thread,
	                        jobject object,
	                        jclass object_klass,
	                        jlong size) override;

private:
	std::set<std::string> _filter;
};
