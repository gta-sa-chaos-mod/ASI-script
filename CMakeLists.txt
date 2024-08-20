cmake_minimum_required(VERSION 3.16)
project(TrilogyChaosMod)

# C++20/17
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

if(MSVC_VERSION GREATER_EQUAL "1900")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_compile_options(/Zc:preprocessor /Zm500)
endif()

# Include CMakes
include(cmake/DirectXSDK.cmake)
include(cmake/pluginSDK.cmake)

include(cmake/nlohmannJSON.cmake)
include(cmake/uWebSockets.cmake)
include(cmake/minhook.cmake)
include(cmake/efsw.cmake)

# include(cmake/gta3.cmake)
# include(cmake/gtaVC.cmake)
include(cmake/gtaSA.cmake)

if(DEFINED ENV{GITHUB_SHA})
  string(SUBSTRING "$ENV{GITHUB_SHA}" 0 7 VERSION_SUFFIX)
  message(STATUS "Short SHA: ${VERSION_SUFFIX}")
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/shared/util/Version.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/shared/util/Version.h")
