cmake_minimum_required(VERSION 3.20..3.30)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(DPP_BUILD_TEST OFF CACHE INTERNAL "")
set(DPP_USE_EXTERNAL_JSON ON CACHE INTERNAL "")
set(JSON_BuildTests OFF CACHE INTERNAL "")

project(mirror-down-detector 
VERSION 1.0
DESCRIPTION "A discord bot"
)

include(cmake/fetch_deps.cmake)

add_subdirectory(src)
