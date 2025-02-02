#include <functional>

#include "profiler.hpp"

#include <mutex>

profiler::profiler(jvmtiEnv *jvmtiEnv) : _jvmtiEnv(jvmtiEnv), _collector(nullptr), _filter(nullptr), _enabled(false), _sample_interval(0)
{
}

profiler::~profiler()
{
	_collector = nullptr;
	_filter = nullptr;
}

// note lifetime is handled by collector owner, not profiler
void profiler::setCollector(collector *collector)
{
	auto existing = _collector;
	_collector = collector;
}

// note lifetime is handled by filter owner, not profiler
void profiler::setFilter(filter *filter)
{
	auto existing = _filter;
	_filter = filter;
}

jvmtiError profiler::enable()
{
	if (_enabled)
		return JVMTI_ERROR_NONE;

	_enabled = true;
	return JVMTI_ERROR_NONE;
}

jvmtiError profiler::disable()
{
	if (!_enabled)
		return JVMTI_ERROR_NONE;

	_enabled = false;
	return JVMTI_ERROR_NONE;
}

jvmtiError profiler::setSampleRate(jint sampling_interval)
{
	_sample_interval = sampling_interval;
	return JVMTI_ERROR_NONE;
}

bool profiler::isEnabled() const {
	return _enabled;
}

void profiler::sampledObjectAlloc(jvmtiEnv *jvmti_env,
								  JNIEnv *jni_env,
								  jthread thread,
								  jobject object,
								  jclass object_klass,
								  jlong size) const {

	auto filter = _filter;
	if (nullptr != filter)
	{
		if (const auto predicate = filter->sampledObjectAlloc(jvmti_env, jni_env, thread, object, object_klass, size); !predicate)
			return;
	}

	if (const auto collector = _collector; nullptr != collector)
	{
		collector->sampledObjectAlloc(jvmti_env, jni_env, thread, object, object_klass, size);
	}
}

profiler *global_profiler = nullptr;
std::mutex profiler_mutex;

void setGlobalProfiler(profiler *profiler)
{
	std::lock_guard lock(profiler_mutex);
	global_profiler = profiler;
}

template <typename T>
T setGlobalProfilerElement(const std::function<T(profiler *)> &func)
{
	std::lock_guard lock(profiler_mutex);

	if constexpr (std::is_void_v<T>)
	{
		func(global_profiler);
		return T();
	}
	else
	{
		return func(global_profiler);
	}
}

profiler *getGlobalProfiler()
{
	std::lock_guard lock(profiler_mutex);
	return global_profiler;
}

void releaseGlobalProfiler()
{
	std::lock_guard lock(profiler_mutex);
	delete global_profiler;
	global_profiler = nullptr;
}

extern "C" JNIEXPORT void Java_dk_stuart_jtestmemprofiler_NativeProfiler_setCollector(JNIEnv *env, jclass klass, jlong nativeHandle)
{
	auto setter = [nativeHandle](profiler *p)
	{
		p->setCollector(reinterpret_cast<collector *>(nativeHandle));
	};
	setGlobalProfilerElement<void>(setter);
}

extern "C" JNIEXPORT void Java_dk_stuart_jtestmemprofiler_NativeProfiler_setFilter(JNIEnv *env, jclass klass, jlong nativeHandle)
{
	auto setter = [nativeHandle](profiler *p)
	{
		p->setFilter(reinterpret_cast<filter *>(nativeHandle));
	};
	setGlobalProfilerElement<void>(setter);
}

extern "C" JNIEXPORT jint Java_dk_stuart_jtestmemprofiler_NativeProfiler_enable(JNIEnv *env, jclass klass)
{
	auto setter = [](profiler *p)
	{
		if (nullptr == p->getCollector())
			return JVMTI_ERROR_INVALID_OBJECT;

		return p->enable();
	};
	return setGlobalProfilerElement<jvmtiError>(setter);
}

extern "C" JNIEXPORT jint Java_dk_stuart_jtestmemprofiler_NativeProfiler_disable(JNIEnv *env, jclass klass)
{
	auto setter = [](profiler *p)
	{
		return p->disable();
	};
	return setGlobalProfilerElement<jvmtiError>(setter);
}

extern "C" JNIEXPORT jboolean Java_dk_stuart_jtestmemprofiler_NativeProfiler_isEnabled(JNIEnv *env, jclass klass)
{
	auto setter = [](profiler *p)
	{
		return p->isEnabled();
	};
	return setGlobalProfilerElement<jboolean>(setter);
}

extern "C" JNIEXPORT jint Java_dk_stuart_jtestmemprofiler_NativeProfiler_setSampleRate(JNIEnv *env, jclass klass, jint sample_interval)
{
	auto setter = [sample_interval](profiler *p)
	{
		return p->setSampleRate(sample_interval);
	};
	return setGlobalProfilerElement<jvmtiError>(setter);
}
