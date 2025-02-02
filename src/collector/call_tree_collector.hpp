#pragma once

#include "trie/trie.hpp"
#include "collector.hpp"

class call_tree_collector final : collector {
  Trie trie;

public:
  const Trie& get_trie();

  void sampledObjectAlloc(jvmtiEnv *jvmti_env,
                          JNIEnv *jni_env,
                          jthread thread,
                          jobject object,
                          jclass object_klass,
                          jlong size) override;
};
