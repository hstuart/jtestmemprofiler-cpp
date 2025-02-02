#include <cstring>
#include <cstdio>

#include "agent.hpp"

extern "C" void JNICALL sampledObjectAlloc(jvmtiEnv *jvmti_env,
										   JNIEnv *jni_env,
										   jthread thread,
										   jobject object,
										   jclass object_klass,
										   jlong size)
{
	auto p = getGlobalProfiler();
	if (nullptr != p)
	{
		if (!p->isEnabled())
		{
			return;
		}

		p->sampledObjectAlloc(jvmti_env, jni_env, thread, object, object_klass, size);
	}
}

extern "C" void JNICALL vmDeath(jvmtiEnv *jvmti_env, JNIEnv *jni_env)
{
	releaseGlobalProfiler();
}

extern "C" JNIEXPORT jint JNICALL Agent_OnLoad(JavaVM *jvm, char *options, void * /* reserved */)
{
	jvmtiEnv *jvmti = nullptr;

	if (auto res = jvm->GetEnv(reinterpret_cast<void **>(&jvmti), JVMTI_VERSION); JNI_OK != res || nullptr == jvmti)
	{
		std::fprintf(stderr, "Unable to get JVMTI interface for version %d\n", JVMTI_VERSION);
		return JNI_ERR;
	}

	jvmtiCapabilities capabilities = {};
	capabilities.can_generate_sampled_object_alloc_events = 1;

	auto error = jvmti->AddCapabilities(&capabilities);
	if (JVMTI_ERROR_NONE != error)
	{
		std::fprintf(stderr, "Unable to add capabilities to JVMTI: %d\n", error);
		return JNI_ERR;
	}

	jvmtiEventCallbacks callbacks = {};
	callbacks.SampledObjectAlloc = &sampledObjectAlloc;
	callbacks.VMDeath = &vmDeath;

	error = jvmti->SetEventCallbacks(&callbacks, sizeof(callbacks));
	if (JVMTI_ERROR_NONE != error)
	{
		fprintf(stderr, "Unable to set callbacks to JVMTI: %d\n", error);
		return error;
	}

	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_SAMPLED_OBJECT_ALLOC, nullptr);
	if (JVMTI_ERROR_NONE != error)
	{
		fprintf(stderr, "Unable to enable sampled object allocation: %d\n", error);
		return JNI_ERR;
	}

	error = jvmti->SetHeapSamplingInterval(0);
	if (JVMTI_ERROR_NONE != error)
	{
		fprintf(stderr, "Unable to set heap sampling interval: %d\n", error);
		return JNI_ERR;
	}

	setGlobalProfiler(new profiler(jvmti));

	return JNI_OK;
}

extern "C" JNIEXPORT void JNICALL Agent_OnUnload(JavaVM *vm)
{
	releaseGlobalProfiler();
}

extern "C" JNIEXPORT jint JNICALL Agent_OnAttach(JavaVM *jvm, char *options, void * /* reserved */)
{
	return Agent_OnLoad(jvm, options, nullptr);
}
