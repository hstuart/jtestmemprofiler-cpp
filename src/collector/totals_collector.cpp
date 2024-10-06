#include "totals_collector.hpp"

totals_collector::totals_collector()
{
	_total.store(0);
}

jlong totals_collector::getTotal()
{
	return _total.load();
}

void totals_collector::sampledObjectAlloc(jvmtiEnv *jvmti_env,
										  JNIEnv *jni_env,
										  jthread thread,
										  jobject object,
										  jclass object_klass,
										  jlong size)
{
	_total += size;
}

extern "C" JNIEXPORT jlong JNICALL Java_dk_stuart_jtestmemprofiler_NativeTotalsCollector_init(JNIEnv *env, jclass klass)
{
	return (jlong) new totals_collector();
}

extern "C" JNIEXPORT void JNICALL Java_dk_stuart_jtestmemprofiler_NativeTotalsCollector_cleanup(JNIEnv *env, jclass klass, jlong nativeHandle)
{
	if (nullptr != (totals_collector *)nativeHandle)
		delete (totals_collector *)nativeHandle;
}

extern "C" JNIEXPORT jlong JNICALL Java_dk_stuart_jtestmemprofiler_NativeTotalsCollector_get(JNIEnv *env, jclass klass, jlong nativeHandle)
{
	auto collector = (totals_collector *)nativeHandle;
	return collector->getTotal();
}
