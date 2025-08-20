include_guard()

function(configure_warnings target)
  set(
    clang_warnings
    -Wall
    -Wextra
    -Wshadow
    -Wcast-align
    -Wunused
    -Wpedantic
    -Wconversion
    -Wno-sign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
    -Wimplicit-fallthrough
    $<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>
    $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>
    $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>
  )

  if(CMAKE_COMPILE_WARNING_AS_ERROR)
    list(APPEND clang_warnings -Werror)
  endif()

  set(
    gcc_warnings
    ${clang_warnings}
    -Wmisleading-indentation
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wuseless-cast
  )

  if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(warnings ${clang_warnings})
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(warnings ${gcc_warnings})
  else()
    message(AUTHOR_WARNING "No compiler warnings set for CXX compiler: ${CMAKE_CXX_COMPILER_ID}")
  endif()

  get_target_property(type ${target} TYPE)

  if(NOT ${type} STREQUAL "INTERFACE_LIBRARY")
    message(AUTHOR_WARNING "Setting compile warnings for non-interface library target: ${target}")
  endif()

  target_compile_options(${target} INTERFACE ${warnings})
endfunction()
