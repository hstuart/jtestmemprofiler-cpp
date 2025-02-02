#pragma once

#include <set>

#include "filter.hpp"

class thread_id_filter final : filter
{
public:
	explicit thread_id_filter(std::set<jlong> filter);

	bool sampledObjectAlloc(jvmtiEnv *jvmti_env,
	                        JNIEnv *jni_env,
	                        jthread thread,
	                        jobject object,
	                        jclass object_klass,
	                        jlong size) override;

private:
	std::set<jlong> _filter;
};
