include_guard()

set(version 0.42.0)

if(CPM_SOURCE_CACHE)
  set(location "${CPM_SOURCE_CACHE}/cpm/CPM_${version}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
  set(location "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${version}.cmake")
else()
  set(location "${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM_${version}.cmake")
endif()

cmake_path(ABSOLUTE_PATH location NORMALIZE OUTPUT_VARIABLE location)

function(download_cpm)
  message(STATUS "Downloading CPM.cmake to: ${location}")
  file(
    DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${location}
    EXPECTED_HASH SHA256=2020b4fc42dba44817983e06342e682ecfc3d2f484a581f11cc5731fbe4dce8a
  )
endfunction()

if(NOT EXISTS ${location})
  download_cpm()
endif()

include(${location})
