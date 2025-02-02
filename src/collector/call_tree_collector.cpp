#include "call_tree_collector.hpp"

#define MAX_FRAME_LEN 1000

const Trie &call_tree_collector::get_trie() {
    return trie;
}

void call_tree_collector::sampledObjectAlloc(jvmtiEnv *jvmti_env,
                                             JNIEnv *jni_env,
                                             const jthread thread,
                                             jobject object,
                                             jclass object_klass,
                                             const jlong size) {
    jvmtiFrameInfo frame_info[MAX_FRAME_LEN];
    jint count;

    if (const auto err = jvmti_env->GetStackTrace(thread, 0, MAX_FRAME_LEN, frame_info, &count); err == JVMTI_ERROR_NONE && count > 0) {
        trie.record(jvmti_env, jni_env, frame_info, count, size);
    }
}

extern "C" JNIEXPORT jlong JNICALL Java_dk_stuart_jtestmemprofiler_NativeCallTreeCollector_init(JNIEnv* env, jclass cls) {
    return reinterpret_cast<jlong>(new call_tree_collector());
}

extern "C" JNIEXPORT void JNICALL Java_dk_stuart_jtestmemprofiler_NativeCallTreeCollector_cleanup(JNIEnv* env, jclass cls, jlong nativeHandle) {
    delete reinterpret_cast<call_tree_collector*>(nativeHandle);
}

jobject bottom_up_allocate(JNIEnv* env, const std::shared_ptr<TrieNode> &node, jclass hash_map, jmethodID hash_map_constructor, jmethodID hash_map_put, jclass trie_node, jmethodID trie_node_constructor) {
    const auto map = env->NewObject(hash_map, hash_map_constructor, static_cast<jint>(node->children.size()));
    for (const auto &[fst, snd] : node->children) {
        const auto child_key = env->NewStringUTF(fst.c_str());
        const auto child_value = bottom_up_allocate(env, snd, hash_map, hash_map_constructor, hash_map_put, trie_node, trie_node_constructor);

        env->CallObjectMethod(map, hash_map_put, child_key, child_value);

        env->DeleteLocalRef(child_key);
        env->DeleteLocalRef(child_value);
    }

    const auto rv = env->NewObject(trie_node, trie_node_constructor, map, node->allocation_size.load(), node->child_accumulated_allocation_size.load());
    return rv;
}

extern "C" JNIEXPORT jobject JNICALL Java_dk_stuart_jtestmemprofiler_NativeCallTreeCollector_get(JNIEnv* env, jclass cls, jlong nativeHandle) {
    const auto collector = reinterpret_cast<call_tree_collector*>(nativeHandle);

    const auto hashMapClass = env->FindClass("java/util/HashMap");
    const auto hashMapConstructor = env->GetMethodID(hashMapClass, "<init>", "(I)V");
    const auto putMethod = env->GetMethodID(hashMapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    const auto trieClass = env->FindClass("dk/stuart/jtestmemprofiler/TrieNode");
    const auto trieConstructor = env->GetMethodID(trieClass, "<init>", "(Ljava/util/HashMap;JJ)V");

    const auto root = collector->get_trie();

    const auto javaRoot = bottom_up_allocate(env, root.getRoot(), hashMapClass, hashMapConstructor, putMethod, trieClass, trieConstructor);

    env->DeleteLocalRef(hashMapClass);
    env->DeleteLocalRef(trieClass);

    return javaRoot;
}
