include_guard()

function(configure_clang_tidy)
  find_program(CLANG_TIDY NAMES clang-tidy clang-tidy-20 REQUIRED)

  list(APPEND CMAKE_MESSAGE_CONTEXT clang-tidy)

  set(
    options
    ${CLANG_TIDY}
    -p
    ${project_root}/compile_commands.json
    -extra-arg=-Wno-unknown-warning-option
    -extra-arg=-Wno-ignored-optimization-argument
    -extra-arg=-Wno-unused-command-line-argument
  )

  if(CMAKE_COMPILE_WARNING_AS_ERROR)
    list(APPEND options -warnings-as-errors=*)
  endif()

  set(c_options ${options})
  set(c++_options ${options})

  if(NOT "${CMAKE_C_STANDARD}" STREQUAL "")
    list(APPEND c_options -extra-arg=-std=c${CMAKE_C_STANDARD})
  endif()

  if(NOT "${CMAKE_CXX_STANDARD}" STREQUAL "")
    list(APPEND c++_options -extra-arg=-std=c++${CMAKE_CXX_STANDARD})
  endif()

  message(STATUS "${c_options}")
  message(STATUS "${c++_options}")

  set(CMAKE_C_CLANG_TIDY ${c_options} PARENT_SCOPE)
  set(CMAKE_CXX_CLANG_TIDY ${c++_options} PARENT_SCOPE)
endfunction()

function(configure_include_what_you_use)
  find_program(INCLUDE_WHAT_YOU_USE include-what-you-use)

  if(NOT INCLUDE_WHAT_YOU_USE)
    message(WARNING "include-what-you-use requested but executable not found")
    return()
  endif()

  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${INCLUDE_WHAT_YOU_USE} -Wno-unknown-warning-option PARENT_SCOPE)
endfunction()
