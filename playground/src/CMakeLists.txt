add_executable(main main.cpp)
target_link_libraries(
  main
  PRIVATE playground::warnings playground::sanitizers fmt::fmt ts::tree_sitter ts::cmake
)
