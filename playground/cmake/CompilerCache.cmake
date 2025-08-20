include_guard()

# See Professional CMake: A Practical Guide 15th Edition, Appendix B
function(configure_ccache)
  if(NOT PROJECT_IS_TOP_LEVEL)
    return()
  endif()

  find_program(CCACHE_PATH ccache)

  if(NOT CCACHE_PATH)
    message(AUTHOR_WARNING "Ccache requested but not found")
    return()
  endif()

  # clang -cc1 needs -fno-pch-timestamp
  # https://ccache.dev/manual/4.7.4.html#_precompiled_headers
  # https://clang.llvm.org/docs/ClangCommandLineReference.html#cmdoption-clang-xclang
  # https://clang.llvm.org/docs/FAQ.html#i-run-clang-cc1-and-get-weird-errors-about-missing-headers
  foreach(lang IN ITEMS C CXX)
    if(CMAKE_${lang}_COMPILER_ID MATCHES "Clang")
      add_compile_options("$<$<COMPILE_LANGUAGE:${lang}>:SHELL:-Xclang -fno-pch-timestamp>")
    endif()
  endforeach()

  set(
    CCACHE_ENV
    CCACHE_SLOPPINESS=pch_defines,time_macros
    CACHE STRING
    "List of environment variables for ccache, each in key=value form"
  )

  if(MSVC)
    foreach(lang IN ITEMS C CXX)
      foreach(config IN LISTS CMAKE_BUILD_TYPE CMAKE_CONFIGURATION_TYPES)
        set(flags CMAKE_${lang}_FLAGS)

        if(NOT config STREQUAL "")
          string(TOUPPER "${config}" config)
          string(APPEND flags "_${config}")
        endif()

        # CCache cannot cache debug information produced by -Zi very well
        # https://github.com/ccache/ccache/issues/1040
        # https://learn.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-by-category#language
        string(REGEX REPLACE "[-/]Z[iI]" "-Z7" ${flags} "${${flags}}")

        set(${flags} "${${flags}}" PARENT_SCOPE)
      endforeach()
    endforeach()

    # New in CMake version 3.25
    # https://cmake.org/cmake/help/latest/variable/CMAKE_MSVC_DEBUG_INFORMATION_FORMAT.html#variable:CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
    if(DEFINED CMAKE_MSVC_DEBUG_INFORMATION_FORMAT)
      string(
        REGEX REPLACE
        "ProgramDatabase|EditAndContinue"
        "Embedded"
        replaced
        "${CMAKE_MSVC_DEBUG_INFORMATION_FORMAT}"
      )
      set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "${replaced}" PARENT_SCOPE)
    else()
      set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>" PARENT_SCOPE)
    endif()
  endif()

  if(CMAKE_GENERATOR MATCHES "Ninja|Makefiles")
    set(
      launcher
      ${CMAKE_COMMAND}
      -E
      env
      ${CCACHE_ENV}
      ${CCACHE_PATH}
    )

    foreach(lang IN ITEMS C CXX)
      set(CMAKE_${lang}_COMPILER_LAUNCHER ${launcher} PARENT_SCOPE)
    endforeach()

    message(STATUS "Ccache compiler launcher: ${launcher}")
  elseif(CMAKE_GENERATOR STREQUAL "Xcode")
    foreach(lang IN ITEMS C CXX)
      list(JOIN CCACHE_ENV "\nexport " set_env)
      if(NOT set_env STREQUAL "")
        string(PREPEND set_env "export ")
      endif()

      set(launch_${lang} ${CMAKE_BINARY_DIR}/launch-${lang})

      file(
        WRITE
        ${launch_${lang}}
        "#!/bin/bash\n"
        "${set_env}\n"
        "exec \"${CCACHE_PATH}\" \"${CMAKE_${lang}_COMPILER}\" \"$@\"\n"
      )

      execute_process(COMMAND chmod a+rx ${launch_${lang}})
    endforeach()

    set(CMAKE_XCODE_ATTRIBUTE_CC ${launch_C} PARENT_SCOPE)
    set(CMAKE_XCODE_ATTRIBUTE_CXX ${launch_CXX} PARENT_SCOPE)
    set(CMAKE_XCODE_ATTRIBUTE_LD ${launch_C} PARENT_SCOPE)
    set(CMAKE_XCODE_ATTRIBUTE_LDPLUSPLUS ${launch_CXX} PARENT_SCOPE)
  elseif(CMAKE_GENERATOR MATCHES "Visual Studio")
    cmake_path(NATIVE_PATH CCACHE_PATH ccache_exe)

    list(JOIN CCACHE_ENV "\nset " set_env)
    if(NOT set_env STREQUAL "")
      string(PREPEND set_env "set ")
    endif()

    get_property(langs GLOBAL PROPERTY ENABLED_LANGUAGES)

    if(CXX IN_LIST langs)
      set(compiler "${CMAKE_CXX_COMPILER}")
    else()
      set(compiler "${CMAKE_C_COMPILER}")
    endif()

    file(WRITE ${CMAKE_BINARY_DIR}/launch-cl.cmd "@echo off\n" "${set_env}\n" "\"${ccache_exe}\" \"${compiler}\" %*\n")

    list(FILTER CMAKE_VS_GLOBALS EXCLUDE REGEX "^(CLTool(Path|Exe)|TrackFileAccess)=.*$")
    list(APPEND CMAKE_VS_GLOBALS CLToolPath=${CMAKE_BINARY_DIR} CLToolExe=launch-cl.cmd TrackFileAccess=false)

    if(NOT CMAKE_VS_GLOBALS MATCHES "(^|;)UseMultiToolTask=")
      list(APPEND CMAKE_VS_GLOBALS UseMultiToolTask=true)
    endif()

    if(NOT CMAKE_VS_GLOBALS MATCHES "(^|;)EnforceProcessCountAcrossBuilds=")
      list(APPEND CMAKE_VS_GLOBALS EnforceProcessCountAcrossBuilds=true)
    endif()

    set(CMAKE_VS_GLOBALS "${CMAKE_VS_GLOBALS}" PARENT_SCOPE)
  endif()
endfunction()
