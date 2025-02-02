#pragma once

#include <jvmti.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <shared_mutex>
#include <atomic>
#include <mutex>

class TrieNode {
public:
    std::unordered_map<std::string, std::shared_ptr<TrieNode>> children;
    std::atomic<jlong> allocation_size{0};
    std::atomic<jlong> child_accumulated_allocation_size{0};
    std::shared_mutex nodeMutex;
};

class Trie {
    std::shared_ptr<TrieNode> root;

public:
    Trie() : root(std::make_shared<TrieNode>()) {}

    [[nodiscard]] std::shared_ptr<TrieNode> const& getRoot() const { return root; }

    void record(jvmtiEnv *jvmti_env, JNIEnv *jni_env, const jvmtiFrameInfo* frame_info, const size_t frameLength, const jlong allocation_size) const {
        std::shared_ptr<TrieNode> current = root;
        for (auto j = frameLength; j > 0; --j) {
            const auto i = j - 1;

            char* method_name;
            char* signature_name;
            char* generic_name;
            char* class_name;
            char* class_generic_name;
            jclass declaring_class;

            if (const auto methodErr = jvmti_env->GetMethodName(frame_info[i].method, &method_name, &signature_name, &generic_name); methodErr != JVMTI_ERROR_NONE) {
                return;
            }
            if (const auto classErr = jvmti_env->GetMethodDeclaringClass(frame_info[i].method, &declaring_class); classErr != JVMTI_ERROR_NONE) {
                jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(method_name));
                jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(signature_name));
                jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(generic_name));
                return;
            }
            if (const auto classNameErr = jvmti_env->GetClassSignature(declaring_class, &class_name, &class_generic_name); classNameErr != JVMTI_ERROR_NONE) {
                jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(method_name));
                jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(signature_name));
                jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(generic_name));
                jni_env->DeleteLocalRef(declaring_class);
                return;
            }

            std::string frameName = class_name;
            if (class_generic_name != nullptr) {
                frameName += class_generic_name;
            }
            frameName += "::";
            frameName += method_name;
            if (generic_name != nullptr) {
                frameName += generic_name;
            }
            frameName += signature_name;

            std::unique_lock lock(current->nodeMutex);
            if (!current->children[frameName]) {
                current->children[frameName] = std::make_shared<TrieNode>();
                current->child_accumulated_allocation_size += allocation_size;
                current = current->children[frameName];
                if (i == 0) {
                    current->allocation_size = allocation_size;
                    current->child_accumulated_allocation_size += allocation_size;
                }
            } else {
                current->child_accumulated_allocation_size += allocation_size;
                current = current->children[frameName];
                if (i == 0) {
                    current->allocation_size += allocation_size;
                    current->child_accumulated_allocation_size += allocation_size;
                }
            }

            jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(method_name));
            jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(signature_name));
            jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(generic_name));
            jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(class_name));
            jvmti_env->Deallocate(reinterpret_cast<unsigned char*>(class_generic_name));
            jni_env->DeleteLocalRef(declaring_class);
        }
    }
};