add_library(tree_sitter_cmake src/parser.c src/scanner.c)
add_library(ts::cmake ALIAS tree_sitter_cmake)
target_include_directories(tree_sitter_cmake INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/include)
