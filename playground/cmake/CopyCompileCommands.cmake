include_guard()

function(copy_compile_commands)
  set(output ${project_root}/../compile_commands.json)
  add_custom_target(copy_compile_commands ALL DEPENDS ${output})
  add_custom_command(
    OUTPUT ${output}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json ${output}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    COMMENT "Copying compile commands to: ${output}"
  )
endfunction()
