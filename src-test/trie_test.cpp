#include "../src/collector/trie/trie.hpp"

#include <array>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stub_jni.hpp"
#include "stub_jvmti.hpp"

using namespace testing;

TEST(TrieTest, EmptyTreeIsInitialized) {
    const Trie trie{};
    ASSERT_NE(trie.getRoot(), nullptr);
}

TEST(TrieTest, RecordSingleStackSuccess) {
    const auto frameInfos = create_frame_infos(0, 2);
    constexpr jlong allocation_size = 1024;

    auto getMethodName = [](jvmtiEnv *env, jmethodID method, char **name_ptr, char **signature, char **generic_ptr) {
        if (method == reinterpret_cast<jmethodID>(1)) {
            *name_ptr = const_cast<char *>("frame0Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else if (method == reinterpret_cast<jmethodID>(2)) {
            *name_ptr = const_cast<char *>("frame1Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else {
            *name_ptr = nullptr;
            *signature = nullptr;
            *generic_ptr = nullptr;
        }
        return JVMTI_ERROR_NONE;
    };

    auto getMethodDeclaringClass = [](jvmtiEnv *env, jmethodID method, jclass *declaring_class_ptr) {
        if (method == reinterpret_cast<jmethodID>(1)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(1));
        } else if (method == reinterpret_cast<jmethodID>(2)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(2));
        } else {
            *declaring_class_ptr = nullptr;
        }

        return JVMTI_ERROR_NONE;
    };

    auto getClassSignature = [](jvmtiEnv *env, jclass klass, char **signature_ptr, char **generic_ptr) {
        if (klass == reinterpret_cast<jclass>(1)) {
            *signature_ptr = const_cast<char *>("com/example/Foo");
            *generic_ptr = nullptr;
        } else if (klass == reinterpret_cast<jclass>(2)) {
            *signature_ptr = const_cast<char *>("com/example/Bar");
            *generic_ptr = nullptr;
        } else {
            *signature_ptr = nullptr;
            *generic_ptr = nullptr;
        }
        return JVMTI_ERROR_NONE;
    };

    auto deallocate = [](jvmtiEnv *env, unsigned char *mem) {
        return JVMTI_ERROR_NONE;
    };

    StubJvmtiFunctions functions{};
    functions.setGetMethodName(getMethodName);
    functions.setGetMethodDeclaringClass(getMethodDeclaringClass);
    functions.setGetClassSignature(getClassSignature);
    functions.setDeallocate(deallocate);
    StubJvmtiEnv jvmtiEnv(&functions);

    StubJniFunctions jniFunctions{};
    jniFunctions.setDeleteLocalRef([](JNIEnv *env, jobject object) {});
    StubJniEnv jniEnv(&jniFunctions);

    const Trie trie{};

    trie.record(&jvmtiEnv, &jniEnv, frameInfos.data(), frameInfos.size(), allocation_size);

    const auto &root = trie.getRoot();
    ASSERT_FALSE(root->children.empty());

    ASSERT_EQ(trie.getRoot()->child_accumulated_allocation_size.load(), allocation_size);
    ASSERT_EQ(trie.getRoot()->allocation_size.load(), 0);
    ASSERT_EQ(trie.getRoot()->children.size(), 1);
    const auto frame1 = trie.getRoot()->children.begin();
    ASSERT_EQ(frame1->first, "com/example/Bar::frame1Method()V");
    ASSERT_EQ(frame1->second->child_accumulated_allocation_size.load(), allocation_size);
    ASSERT_EQ(frame1->second->allocation_size.load(), 0);
    ASSERT_EQ(frame1->second->children.size(), 1);
    const auto frame2 = frame1->second->children.begin();
    ASSERT_EQ(frame2->first, "com/example/Foo::frame0Method()V");
    ASSERT_EQ(frame2->second->child_accumulated_allocation_size.load(), allocation_size);
    ASSERT_EQ(frame2->second->allocation_size.load(), allocation_size);
    ASSERT_EQ(frame2->second->children.size(), 0);
}

TEST(TrieTest, RecordSingleStackWithGenericsSuccess) {
    const auto frameInfos = create_frame_infos(0, 2);
    constexpr jlong allocation_size = 1024;

    auto getMethodName = [](jvmtiEnv *env, jmethodID method, char **name_ptr, char **signature, char **generic_ptr) {
        if (method == reinterpret_cast<jmethodID>(1)) {
            *name_ptr = const_cast<char *>("frame0Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = const_cast<char *>("");
        } else if (method == reinterpret_cast<jmethodID>(2)) {
            *name_ptr = const_cast<char *>("frame1Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else {
            *name_ptr = nullptr;
            *signature = nullptr;
            *generic_ptr = nullptr;
        }
        return JVMTI_ERROR_NONE;
    };

    auto getMethodDeclaringClass = [](jvmtiEnv *env, jmethodID method, jclass *declaring_class_ptr) {
        if (method == reinterpret_cast<jmethodID>(1)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(1));
        } else if (method == reinterpret_cast<jmethodID>(2)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(2));
        } else {
            *declaring_class_ptr = nullptr;
        }

        return JVMTI_ERROR_NONE;
    };

    auto getClassSignature = [](jvmtiEnv *env, jclass klass, char **signature_ptr, char **generic_ptr) {
        if (klass == reinterpret_cast<jclass>(1)) {
            *signature_ptr = const_cast<char *>("com/example/Foo");
            *generic_ptr = nullptr;
        } else if (klass == reinterpret_cast<jclass>(2)) {
            *signature_ptr = const_cast<char *>("com/example/Bar");
            *generic_ptr = nullptr;
        } else {
            *signature_ptr = nullptr;
            *generic_ptr = nullptr;
        }
        return JVMTI_ERROR_NONE;
    };

    auto deallocate = [](jvmtiEnv *env, unsigned char *mem) {
        return JVMTI_ERROR_NONE;
    };

    StubJvmtiFunctions functions{};
    functions.setGetMethodName(getMethodName);
    functions.setGetMethodDeclaringClass(getMethodDeclaringClass);
    functions.setGetClassSignature(getClassSignature);
    functions.setDeallocate(deallocate);
    StubJvmtiEnv jvmtiEnv(&functions);

    StubJniFunctions jniFunctions{};
    jniFunctions.setDeleteLocalRef([](JNIEnv *env, jobject object) {});
    StubJniEnv jniEnv(&jniFunctions);

    const Trie trie{};

    trie.record(&jvmtiEnv, &jniEnv, frameInfos.data(), frameInfos.size(), allocation_size);

    const auto &root = trie.getRoot();
    ASSERT_FALSE(root->children.empty());

    ASSERT_EQ(trie.getRoot()->child_accumulated_allocation_size.load(), allocation_size);
    ASSERT_EQ(trie.getRoot()->allocation_size.load(), 0);
    ASSERT_EQ(trie.getRoot()->children.size(), 1);
    const auto frame1 = trie.getRoot()->children.begin();
    ASSERT_EQ(frame1->first, "com/example/Bar::frame1Method()V");
    ASSERT_EQ(frame1->second->child_accumulated_allocation_size.load(), allocation_size);
    ASSERT_EQ(frame1->second->allocation_size.load(), 0);
    ASSERT_EQ(frame1->second->children.size(), 1);
    const auto frame2 = frame1->second->children.begin();
    ASSERT_EQ(frame2->first, "com/example/Foo::frame0Method()V");
    ASSERT_EQ(frame2->second->child_accumulated_allocation_size.load(), allocation_size);
    ASSERT_EQ(frame2->second->allocation_size.load(), allocation_size);
    ASSERT_EQ(frame2->second->children.size(), 0);
}

TEST(TrieTest, RecordMultipleRootFrameSuccess) {
    auto frameInfos1 = create_frame_infos(0, 2);
    auto frameInfos2 = create_frame_infos(2, 2);
    constexpr jlong allocation_size = 1024;

    auto getMethodName = [](jvmtiEnv *env, jmethodID method, char **name_ptr, char **signature, char **generic_ptr) {
        if (method == reinterpret_cast<jmethodID>(1)) {
            *name_ptr = const_cast<char *>("frame0Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else if (method == reinterpret_cast<jmethodID>(2)) {
            *name_ptr = const_cast<char *>("frame1Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else if (method == reinterpret_cast<jmethodID>(3)) {
            *name_ptr = const_cast<char *>("frame2Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else if (method == reinterpret_cast<jmethodID>(4) ) {
            *name_ptr = const_cast<char *>("frame3Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else {
            *name_ptr = nullptr;
            *signature = nullptr;
            *generic_ptr = nullptr;
        }
        return JVMTI_ERROR_NONE;
    };

    auto getMethodDeclaringClass = [](jvmtiEnv *env, jmethodID method, jclass *declaring_class_ptr) {
        if (method == reinterpret_cast<jmethodID>(1)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(1));
        } else if (method == reinterpret_cast<jmethodID>(2)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(2));
        } else if (method == reinterpret_cast<jmethodID>(3)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(3));
        } else if (method == reinterpret_cast<jmethodID>(4)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(4));
        } else {
            *declaring_class_ptr = nullptr;
        }

        return JVMTI_ERROR_NONE;
    };

    auto getClassSignature = [](jvmtiEnv *env, jclass klass, char **signature_ptr, char **generic_ptr) {
        if (klass == reinterpret_cast<jclass>(1)) {
            *signature_ptr = const_cast<char *>("com/example/Foo");
            *generic_ptr = nullptr;
        } else if (klass == reinterpret_cast<jclass>(2)) {
            *signature_ptr = const_cast<char *>("com/example/Bar");
            *generic_ptr = nullptr;
        } else if (klass == reinterpret_cast<jclass>(3)) {
            *signature_ptr = const_cast<char *>("com/example/Baz");
            *generic_ptr = nullptr;
        } else if (klass == reinterpret_cast<jclass>(4)) {
            *signature_ptr = const_cast<char *>("com/example/Quux");
            *generic_ptr = nullptr;
        } else {
            *signature_ptr = nullptr;
            *generic_ptr = nullptr;
        }
        return JVMTI_ERROR_NONE;
    };

    auto deallocate = [](jvmtiEnv *env, unsigned char *mem) {
        return JVMTI_ERROR_NONE;
    };

    StubJvmtiFunctions functions{};
    functions.setGetMethodName(getMethodName);
    functions.setGetMethodDeclaringClass(getMethodDeclaringClass);
    functions.setGetClassSignature(getClassSignature);
    functions.setDeallocate(deallocate);
    StubJvmtiEnv jvmtiEnv(&functions);

    StubJniFunctions jniFunctions{};
    jniFunctions.setDeleteLocalRef([](JNIEnv *env, jobject object) {});
    StubJniEnv jniEnv(&jniFunctions);

    const Trie trie{};

    trie.record(&jvmtiEnv, &jniEnv, frameInfos1.data(), frameInfos1.size(), allocation_size);
    trie.record(&jvmtiEnv, &jniEnv, frameInfos2.data(), frameInfos2.size(), allocation_size);

    const auto &root = trie.getRoot();
    ASSERT_FALSE(root->children.empty());

    ASSERT_EQ(trie.getRoot()->child_accumulated_allocation_size.load(), 2 * allocation_size);
    ASSERT_EQ(trie.getRoot()->allocation_size.load(), 0);
    ASSERT_EQ(trie.getRoot()->children.size(), 2);

    const auto frame1 = trie.getRoot()->children["com/example/Bar::frame1Method()V"];
    ASSERT_EQ(frame1->child_accumulated_allocation_size.load(), allocation_size);
    ASSERT_EQ(frame1->allocation_size.load(), 0);
    ASSERT_EQ(frame1->children.size(), 1);
    const auto frame2 = frame1->children["com/example/Foo::frame0Method()V"];
    ASSERT_EQ(frame2->child_accumulated_allocation_size.load(), allocation_size);
    ASSERT_EQ(frame2->allocation_size.load(), allocation_size);
    ASSERT_EQ(frame2->children.size(), 0);

    const auto frame3 = trie.getRoot()->children["com/example/Quux::frame3Method()V"];
    ASSERT_EQ(frame3->child_accumulated_allocation_size.load(), allocation_size);
    ASSERT_EQ(frame3->allocation_size.load(), 0);
    ASSERT_EQ(frame3->children.size(), 1);
    const auto frame4 = frame3->children["com/example/Baz::frame2Method()V"];
    ASSERT_EQ(frame4->child_accumulated_allocation_size.load(), allocation_size);
    ASSERT_EQ(frame4->allocation_size.load(), allocation_size);
    ASSERT_EQ(frame4->children.size(), 0);
}

template<typename... Args>
std::vector<jvmtiFrameInfo> create_frame_infos_from_args(Args... ints) {
    std::vector<jvmtiFrameInfo> frames(sizeof...(ints));
    std::array<int, sizeof...(ints)> offsets = {ints...};
    for (int i = 0; i < offsets.size(); ++i) {
        frames[i].method = reinterpret_cast<jmethodID>(static_cast<intptr_t>(offsets[i]));
    }
    return frames;
}

TEST(TrieTest, RecordPartialOverlappingStackSuccess) {
    auto frameInfos1 = create_frame_infos_from_args(1, 2);
    auto frameInfos2 = create_frame_infos_from_args(3, 2);
    constexpr jlong allocation_size = 1024;

    auto getMethodName = [](jvmtiEnv *env, jmethodID method, char **name_ptr, char **signature, char **generic_ptr) {
        if (method == reinterpret_cast<jmethodID>(1)) {
            *name_ptr = const_cast<char *>("frame0Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else if (method == reinterpret_cast<jmethodID>(2)) {
            *name_ptr = const_cast<char *>("frame1Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else if (method == reinterpret_cast<jmethodID>(3)) {
            *name_ptr = const_cast<char *>("frame2Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else if (method == reinterpret_cast<jmethodID>(4) ) {
            *name_ptr = const_cast<char *>("frame3Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else {
            *name_ptr = nullptr;
            *signature = nullptr;
            *generic_ptr = nullptr;
        }
        return JVMTI_ERROR_NONE;
    };

    auto getMethodDeclaringClass = [](jvmtiEnv *env, jmethodID method, jclass *declaring_class_ptr) {
        if (method == reinterpret_cast<jmethodID>(1)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(1));
        } else if (method == reinterpret_cast<jmethodID>(2)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(2));
        } else if (method == reinterpret_cast<jmethodID>(3)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(3));
        } else if (method == reinterpret_cast<jmethodID>(4)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(4));
        } else {
            *declaring_class_ptr = nullptr;
        }

        return JVMTI_ERROR_NONE;
    };

    auto getClassSignature = [](jvmtiEnv *env, jclass klass, char **signature_ptr, char **generic_ptr) {
        if (klass == reinterpret_cast<jclass>(1)) {
            *signature_ptr = const_cast<char *>("com/example/Foo");
            *generic_ptr = nullptr;
        } else if (klass == reinterpret_cast<jclass>(2)) {
            *signature_ptr = const_cast<char *>("com/example/Bar");
            *generic_ptr = nullptr;
        } else if (klass == reinterpret_cast<jclass>(3)) {
            *signature_ptr = const_cast<char *>("com/example/Baz");
            *generic_ptr = nullptr;
        } else if (klass == reinterpret_cast<jclass>(4)) {
            *signature_ptr = const_cast<char *>("com/example/Quux");
            *generic_ptr = nullptr;
        } else {
            *signature_ptr = nullptr;
            *generic_ptr = nullptr;
        }
        return JVMTI_ERROR_NONE;
    };

    auto deallocate = [](jvmtiEnv *env, unsigned char *mem) {
        return JVMTI_ERROR_NONE;
    };

    StubJvmtiFunctions functions{};
    functions.setGetMethodName(getMethodName);
    functions.setGetMethodDeclaringClass(getMethodDeclaringClass);
    functions.setGetClassSignature(getClassSignature);
    functions.setDeallocate(deallocate);
    StubJvmtiEnv jvmtiEnv(&functions);

    StubJniFunctions jniFunctions{};
    jniFunctions.setDeleteLocalRef([](JNIEnv *env, jobject object) {});
    StubJniEnv jniEnv(&jniFunctions);

    const Trie trie{};

    trie.record(&jvmtiEnv, &jniEnv, frameInfos1.data(), frameInfos1.size(), allocation_size);
    trie.record(&jvmtiEnv, &jniEnv, frameInfos2.data(), frameInfos2.size(), allocation_size);

    const auto &root = trie.getRoot();
    ASSERT_FALSE(root->children.empty());

    ASSERT_EQ(trie.getRoot()->child_accumulated_allocation_size.load(), 2 * allocation_size);
    ASSERT_EQ(trie.getRoot()->allocation_size.load(), 0);
    ASSERT_EQ(trie.getRoot()->children.size(), 1);

    const auto frame1 = trie.getRoot()->children["com/example/Bar::frame1Method()V"];
    ASSERT_EQ(frame1->child_accumulated_allocation_size.load(), 2 * allocation_size);
    ASSERT_EQ(frame1->allocation_size.load(), 0);
    ASSERT_EQ(frame1->children.size(), 2);

    const auto frame2 = frame1->children["com/example/Foo::frame0Method()V"];
    ASSERT_EQ(frame2->child_accumulated_allocation_size.load(), allocation_size);
    ASSERT_EQ(frame2->allocation_size.load(), allocation_size);
    ASSERT_EQ(frame2->children.size(), 0);

    const auto frame3 = frame1->children["com/example/Baz::frame2Method()V"];
    ASSERT_EQ(frame3->child_accumulated_allocation_size.load(), allocation_size);
    ASSERT_EQ(frame3->allocation_size.load(), allocation_size);
    ASSERT_EQ(frame3->children.size(), 0);
}

TEST(TrieTest, RecordFullyOverlappingStackSuccess) {
    constexpr int frameLength = 2;
    auto frameInfos1 = create_frame_infos_from_args(1, 2);
    constexpr jlong allocation_size = 1024;

    auto getMethodName = [](jvmtiEnv *env, jmethodID method, char **name_ptr, char **signature, char **generic_ptr) {
        if (method == reinterpret_cast<jmethodID>(1)) {
            *name_ptr = const_cast<char *>("frame0Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else if (method == reinterpret_cast<jmethodID>(2)) {
            *name_ptr = const_cast<char *>("frame1Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else {
            *name_ptr = nullptr;
            *signature = nullptr;
            *generic_ptr = nullptr;
        }
        return JVMTI_ERROR_NONE;
    };

    auto getMethodDeclaringClass = [](jvmtiEnv *env, jmethodID method, jclass *declaring_class_ptr) {
        if (method == reinterpret_cast<jmethodID>(1)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(1));
        } else if (method == reinterpret_cast<jmethodID>(2)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(2));
        } else {
            *declaring_class_ptr = nullptr;
        }

        return JVMTI_ERROR_NONE;
    };

    auto getClassSignature = [](jvmtiEnv *env, jclass klass, char **signature_ptr, char **generic_ptr) {
        if (klass == reinterpret_cast<jclass>(1)) {
            *signature_ptr = const_cast<char *>("com/example/Foo");
            *generic_ptr = nullptr;
        } else if (klass == reinterpret_cast<jclass>(2)) {
            *signature_ptr = const_cast<char *>("com/example/Bar");
            *generic_ptr = nullptr;
        } else {
            *signature_ptr = nullptr;
            *generic_ptr = nullptr;
        }
        return JVMTI_ERROR_NONE;
    };

    auto deallocate = [](jvmtiEnv *env, unsigned char *mem) {
        return JVMTI_ERROR_NONE;
    };

    StubJvmtiFunctions functions{};
    functions.setGetMethodName(getMethodName);
    functions.setGetMethodDeclaringClass(getMethodDeclaringClass);
    functions.setGetClassSignature(getClassSignature);
    functions.setDeallocate(deallocate);
    StubJvmtiEnv jvmtiEnv(&functions);

    StubJniFunctions jniFunctions{};
    jniFunctions.setDeleteLocalRef([](JNIEnv *env, jobject object) {});
    StubJniEnv jniEnv(&jniFunctions);

    const Trie trie{};

    trie.record(&jvmtiEnv, &jniEnv, frameInfos1.data(), frameLength, allocation_size);
    trie.record(&jvmtiEnv, &jniEnv, frameInfos1.data(), frameLength, allocation_size);

    const auto &root = trie.getRoot();
    ASSERT_FALSE(root->children.empty());

    ASSERT_EQ(trie.getRoot()->child_accumulated_allocation_size.load(), 2 * allocation_size);
    ASSERT_EQ(trie.getRoot()->allocation_size.load(), 0);
    ASSERT_EQ(trie.getRoot()->children.size(), 1);

    const auto frame1 = trie.getRoot()->children["com/example/Bar::frame1Method()V"];
    ASSERT_EQ(frame1->child_accumulated_allocation_size.load(), 2 * allocation_size);
    ASSERT_EQ(frame1->allocation_size.load(), 0);
    ASSERT_EQ(frame1->children.size(), 1);

    const auto frame2 = frame1->children["com/example/Foo::frame0Method()V"];
    ASSERT_EQ(frame2->child_accumulated_allocation_size.load(), 2 * allocation_size);
    ASSERT_EQ(frame2->allocation_size.load(), 2 * allocation_size);
    ASSERT_EQ(frame2->children.size(), 0);
}

TEST(TrieTest, RecordFirstChildThenParentAllocationStackSuccess) {
    auto frameInfos1 = create_frame_infos_from_args(1, 2);
    auto frameInfos2 = create_frame_infos_from_args(2);
    constexpr jlong allocation_size = 1024;

    auto getMethodName = [](jvmtiEnv *env, jmethodID method, char **name_ptr, char **signature, char **generic_ptr) {
        if (method == reinterpret_cast<jmethodID>(1)) {
            *name_ptr = const_cast<char *>("frame0Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else if (method == reinterpret_cast<jmethodID>(2)) {
            *name_ptr = const_cast<char *>("frame1Method");
            *signature = const_cast<char *>("()V");
            *generic_ptr = nullptr;
        } else {
            *name_ptr = nullptr;
            *signature = nullptr;
            *generic_ptr = nullptr;
        }
        return JVMTI_ERROR_NONE;
    };

    auto getMethodDeclaringClass = [](jvmtiEnv *env, jmethodID method, jclass *declaring_class_ptr) {
        if (method == reinterpret_cast<jmethodID>(1)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(1));
        } else if (method == reinterpret_cast<jmethodID>(2)) {
            *declaring_class_ptr = reinterpret_cast<jclass>(static_cast<intptr_t>(2));
        } else {
            *declaring_class_ptr = nullptr;
        }

        return JVMTI_ERROR_NONE;
    };

    auto getClassSignature = [](jvmtiEnv *env, jclass klass, char **signature_ptr, char **generic_ptr) {
        if (klass == reinterpret_cast<jclass>(1)) {
            *signature_ptr = const_cast<char *>("com/example/Foo");
            *generic_ptr = nullptr;
        } else if (klass == reinterpret_cast<jclass>(2)) {
            *signature_ptr = const_cast<char *>("com/example/Bar");
            *generic_ptr = nullptr;
        } else {
            *signature_ptr = nullptr;
            *generic_ptr = nullptr;
        }
        return JVMTI_ERROR_NONE;
    };

    auto deallocate = [](jvmtiEnv *env, unsigned char *mem) {
        return JVMTI_ERROR_NONE;
    };

    StubJvmtiFunctions functions{};
    functions.setGetMethodName(getMethodName);
    functions.setGetMethodDeclaringClass(getMethodDeclaringClass);
    functions.setGetClassSignature(getClassSignature);
    functions.setDeallocate(deallocate);
    StubJvmtiEnv jvmtiEnv(&functions);

    StubJniFunctions jniFunctions{};
    jniFunctions.setDeleteLocalRef([](JNIEnv *env, jobject object) {});
    StubJniEnv jniEnv(&jniFunctions);

    const Trie trie{};

    trie.record(&jvmtiEnv, &jniEnv, frameInfos1.data(), frameInfos1.size(), allocation_size);
    trie.record(&jvmtiEnv, &jniEnv, frameInfos2.data(), frameInfos2.size(), allocation_size);

    const auto &root = trie.getRoot();
    ASSERT_FALSE(root->children.empty());

    ASSERT_EQ(trie.getRoot()->child_accumulated_allocation_size.load(), 2 * allocation_size);
    ASSERT_EQ(trie.getRoot()->allocation_size.load(), 0);
    ASSERT_EQ(trie.getRoot()->children.size(), 1);

    const auto frame1 = trie.getRoot()->children["com/example/Bar::frame1Method()V"];
    ASSERT_EQ(frame1->child_accumulated_allocation_size.load(), 2 * allocation_size);
    ASSERT_EQ(frame1->allocation_size.load(), allocation_size);
    ASSERT_EQ(frame1->children.size(), 1);

    const auto frame2 = frame1->children["com/example/Foo::frame0Method()V"];
    ASSERT_EQ(frame2->child_accumulated_allocation_size.load(), allocation_size);
    ASSERT_EQ(frame2->allocation_size.load(), allocation_size);
    ASSERT_EQ(frame2->children.size(), 0);
}
