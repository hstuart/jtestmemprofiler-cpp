#pragma once

#include <mutex>

#include "jvmti.h"

#include "collector/collector.hpp"
#include "filter/filter.hpp"

class profiler
{
public:
	profiler(jvmtiEnv *jvmtiEnv);
	~profiler();

	void setCollector(collector *collector);
	void setFilter(filter *filter);

	jvmtiError enable();
	jvmtiError disable();
	bool isEnabled();

	jvmtiError setSampleRate(jint sampling_interval);

	void sampledObjectAlloc(jvmtiEnv *jvmti_env,
							JNIEnv *jni_env,
							jthread thread,
							jobject object,
							jclass object_klass,
							jlong size);

	collector *getCollector() { return _collector; }
	filter *getFilter() { return _filter; }

private:
	jint _sample_interval;
	bool _enabled;
	collector *_collector;
	filter *_filter;
	jvmtiEnv *_jvmtiEnv;
};

extern void setGlobalProfiler(profiler *profiler);
extern profiler *getGlobalProfiler();
extern void releaseGlobalProfiler();
