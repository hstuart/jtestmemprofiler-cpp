#pragma once

#include <atomic>

#include "collector.hpp"

class totals_collector : collector
{
public:
	totals_collector();

	jlong getTotal();

	virtual void sampledObjectAlloc(jvmtiEnv *jvmti_env,
									JNIEnv *jni_env,
									jthread thread,
									jobject object,
									jclass object_klass,
									jlong size);

private:
	std::atomic<jlong> _total;
};
