unset(CMAKE_C_CLANG_TIDY)
unset(CMAKE_CXX_CLANG_TIDY)

unset(CMAKE_C_INCLUDE_WHAT_YOU_USE)
unset(CMAKE_CXX_INCLUDE_WHAT_YOU_USE)

set_directory_properties(PROPERTIES SYSTEM ON)

include(CPM)

cpmusepackagelock(deps.cmake)

cpmgetpackage(fmt)

cpmgetpackage(tree_sitter)

if(tree_sitter_ADDED)
  add_library(tree_sitter ${tree_sitter_SOURCE_DIR}/lib/src/lib.c)
  add_library(ts::tree_sitter ALIAS tree_sitter)
  target_include_directories(
    tree_sitter
    PRIVATE ${tree_sitter_SOURCE_DIR}/lib/src
    PUBLIC ${tree_sitter_SOURCE_DIR}/lib/include
  )
  set_target_properties(tree_sitter PROPERTIES C_EXTENSIONS ON)
endif()
