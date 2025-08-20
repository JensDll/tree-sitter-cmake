function(configure_sanitizers)
  set(p ${CMAKE_CURRENT_FUNCTION})
  set(options DIRECTORY)
  set(
    single_value_args
    ADDRESS
    LEAK
    UNDEFINED
    MEMORY
    THREAD
    TARGET
  )
  set(multi_value_args)
  cmake_parse_arguments(PARSE_ARGV 0 ${p} "${options}" "${single_value_args}" "${multi_value_args}")

  set(sanitizers "")

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    if(${p}_ADDRESS)
      list(APPEND sanitizers address)
    endif()

    if(${p}_LEAK)
      list(APPEND sanitizers leak)
    endif()

    if(${p}_UNDEFINED)
      list(APPEND sanitizers undefined)
    endif()

    if(${p}_THREAD)
      if(address IN_LIST sanitizers OR leak IN_LIST sanitizers)
        message(WARNING "Thread sanitizer does not work with address/leak sanitizer")
      else()
        list(APPEND sanitizers thread)
      endif()
    endif()

    if(${p}_MEMORY)
      if(address IN_LIST sanitizers OR thread IN_LIST sanitizers OR leak IN_LIST sanitizers)
        message(WARNING "Memory sanitizer does not work with address/thread/leak sanitizer")
      else()
        list(APPEND sanitizers "memory")
      endif()
    endif()
  elseif(MSVC)
    if(${p}_ADDRESS)
      list(APPEND sanitizers "address")
    endif()

    if(${p}_LEAK OR ${p}_UNDEFINED OR ${p}_THREAD OR ${p}_MEMORY)
      message(WARNING "MSVC only supports address sanitizer")
    endif()
  endif()

  list(JOIN sanitizers "," sanitizers)

  if(NOT sanitizers)
    return()
  endif()

  if(NOT MSVC)
    if(${p}_DIRECTORY)
      add_compile_options(-fsanitize=${sanitizers} -fno-sanitize=alignment)
      add_link_options(-fsanitize=${sanitizers} -fno-sanitize=alignment)
    endif()

    if(${p}_TARGET)
      target_compile_options(${${p}_TARGET} INTERFACE -fsanitize=${sanitizers} -fno-sanitize=alignment)
      target_link_options(${${p}_TARGET} INTERFACE -fsanitize=${sanitizers} -fno-sanitize=alignment)
    endif()

    return()
  endif()

  string(FIND "$ENV{PATH}" "$ENV{VSINSTALLDIR}" visual_studio)
  if(visual_studio EQUAL -1)
    message(FATAL_ERROR "Using MSVC sanitizers requires building from the MSVC command prompt")
  endif()

  if(${p}_DIRECTORY)
    add_compile_definitions(_DISABLE_VECTOR_ANNOTATION)
    add_compile_options(/fsanitize=${sanitizers})
    add_link_options(/INCREMENTAL:NO)
  endif()

  if(${p}_TARGET)
    target_compile_definitions(${target} INTERFACE _DISABLE_VECTOR_ANNOTATION)
    target_compile_options(${target} INTERFACE /fsanitize=${sanitizers})
    target_link_options(${target} INTERFACE /INCREMENTAL:NO)
  endif()
endfunction()
