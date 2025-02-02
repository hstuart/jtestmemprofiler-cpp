#pragma once

#include <atomic>
#include <map>
#include <mutex>
#include <string>

#include "collector.hpp"

class per_type_collector final : collector
{
public:
	const std::map<std::string, jlong> &getAllocations();

	void sampledObjectAlloc(jvmtiEnv *jvmti_env,
	                        JNIEnv *jni_env,
	                        jthread thread,
	                        jobject object,
	                        jclass object_klass,
	                        jlong size) override;

private:
	std::mutex _m;
	std::map<std::string, jlong> _allocations;
};