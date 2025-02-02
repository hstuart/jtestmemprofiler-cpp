#pragma once

#include <atomic>

#include "collector.hpp"

class totals_collector final : collector
{
public:
	totals_collector();

	jlong getTotal() const;

	void sampledObjectAlloc(jvmtiEnv *jvmti_env,
	                        JNIEnv *jni_env,
	                        jthread thread,
	                        jobject object,
	                        jclass object_klass,
	                        jlong size) override;

private:
	std::atomic<jlong> _total;
};
