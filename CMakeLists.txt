cmake_minimum_required (VERSION 3.8)

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

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JTestMemProfiler PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(JTestMemProfiler PRIVATE ${JNI_INCLUDE_DIRS})
target_link_libraries(JTestMemProfiler PRIVATE ${JNI_LIBRARIES})
