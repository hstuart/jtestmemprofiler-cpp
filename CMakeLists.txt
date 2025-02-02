cmake_minimum_required (VERSION 3.12)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("JTestMemProfiler")

find_package(JNI REQUIRED)

file(GLOB_RECURSE SOURCES
  ${CMAKE_SOURCE_DIR}/src/**.cpp
  ${CMAKE_SOURCE_DIR}/src/**.hpp
  ${CMAKE_SOURCE_DIR}/src/**.h
)

if(MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /D_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR")
endif()

# Add source to this project's executable.
add_library(JTestMemProfiler SHARED ${SOURCES})

set_property(TARGET JTestMemProfiler PROPERTY CXX_STANDARD 20)

target_include_directories(JTestMemProfiler PRIVATE ${JNI_INCLUDE_DIRS})
target_link_libraries(JTestMemProfiler PRIVATE ${JNI_LIBRARIES})

option(BUILD_TESTS "Build the Google Test suite" OFF)

if (BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
          googletest
          GIT_REPOSITORY https://github.com/google/googletest.git
          GIT_TAG v1.15.2
  )

  FetchContent_GetProperties(googletest)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  file(GLOB_RECURSE TEST_SOURCES
          ${CMAKE_SOURCE_DIR}/src-test/**.cpp
          ${CMAKE_SOURCE_DIR}/src-test/**.hpp
          ${CMAKE_SOURCE_DIR}/src-test/**.h
  )

  add_executable(tests ${TEST_SOURCES})
  set_property(TARGET tests PROPERTY CXX_STANDARD 20)

  target_link_libraries(tests gtest gmock gtest_main ${JNI_LIBRARIES})
  target_include_directories(tests PRIVATE ${JNI_INCLUDE_DIRS})
  enable_testing()
endif ()
