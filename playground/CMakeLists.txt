cmake_minimum_required(VERSION 3.24)

project(Playground VERSION 0.1.0 LANGUAGES C CXX)

set(project_root ${CMAKE_CURRENT_LIST_DIR})
list(APPEND CMAKE_MODULE_PATH ${project_root}/cmake)

include(CTest)
include(CompilerWarnings)
include(StaticAnalyzers)
include(CompilerCache)
include(Sanitizers)
include(CopyCompileCommands)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_VISIBILITY_PRESET hidden)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_LINK_LIBRARIES_ONLY_TARGETS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(playground_warnings INTERFACE)
add_library(playground::warnings ALIAS playground_warnings)
configure_warnings(playground_warnings)

option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
if(ENABLE_CLANG_TIDY)
  configure_clang_tidy()
endif()

option(ENABLE_INCLUDE_WHAT_YOU_USE "Enable include-what-you-use" OFF)
if(ENABLE_INCLUDE_WHAT_YOU_USE)
  configure_include_what_you_use()
endif()

option(ENABLE_CCACHE "Enable ccache" OFF)
if(ENABLE_CCACHE)
  configure_ccache()
endif()

option(ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
option(ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
option(ENABLE_SANITIZER_UNDEFINED "Enable undefined behavior sanitizer" OFF)
option(ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
option(ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)

add_library(playground_sanitizers INTERFACE)
add_library(playground::sanitizers ALIAS playground_sanitizers)
configure_sanitizers(
  ADDRESS ${ENABLE_SANITIZER_ADDRESS}
  LEAK ${ENABLE_SANITIZER_LEAK}
  UNDEFINED ${ENABLE_SANITIZER_UNDEFINED}
  THREAD ${ENABLE_SANITIZER_THREAD}
  MEMORY ${ENABLE_SANITIZER_MEMORY}
  TARGET playground_sanitizers
)

copy_compile_commands()

add_subdirectory(deps)

add_subdirectory(.. tree-sitter-cmake)
target_link_libraries(tree_sitter_cmake PRIVATE playground::warnings playground::sanitizers)

add_subdirectory(src)
