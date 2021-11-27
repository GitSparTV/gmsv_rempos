cmake_minimum_required(VERSION 3.11)

# Creating
project(gmsv_rempos VERSION 2.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

# Config
set(GMOD_MODULE_BASE CACHE STRING "gmod-module-base directory")
set(WEBSOCKETPP CACHE STRING "websocketpp directory")
set(ASIO CACHE STRING "asio directory")
set(ASIO_STANDALONE ON CACHE BOOL "Use standalone asio or boost asio")

# Require paths to be provided
if(NOT GMOD_MODULE_BASE)
    message(FATAL_ERROR "gmod-module-base directory path required. -DGMOD_MODULE_BASE=<dir>")
endif()

if(NOT WEBSOCKETPP)
    message(FATAL_ERROR "websocketpp directory path required. -DWEBSOCKETPP=<dir>")
endif()

if(NOT ASIO)
    message(FATAL_ERROR "asio directory path required. -DASIO=<dir>")
endif()

# RemPos
## Sources
set(REMPOS_GLUA_WRAPPER gmsv_rempos.cpp)
set(REMPOS_INTERNAL rempos.h)
set(REMPOS_JSON json.cpp json.h)
set(REMPOS_SENZOR_SERVER senzor_server.cpp senzor_server.h)

## Target
add_library(rempos SHARED ${REMPOS_INTERNAL} ${REMPOS_GLUA_WRAPPER} ${REMPOS_JSON} ${REMPOS_SENZOR_SERVER})

## CMake includes
add_subdirectory("${GMOD_MODULE_BASE}/include" gmod_module_base_build)

## Include
target_include_directories(rempos PRIVATE ${WEBSOCKETPP} "${ASIO}/include")

## Definitions
if(ASIO_STANDALONE) # If you want boost asio instead, include it manually
	target_compile_definitions(rempos PRIVATE ASIO_STANDALONE _WEBSOCKETPP_CPP11_TYPE_TRAITS_)
endif()
target_compile_definitions(rempos PRIVATE REMPOS_VERSION="${CMAKE_PROJECT_VERSION}")

## Libraries
target_link_libraries(rempos gmod-module-base)

## GMod module specific
set_gmod_suffix_prefix(rempos)