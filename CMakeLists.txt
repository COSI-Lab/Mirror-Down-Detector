cmake_minimum_required(VERSION 3.20...3.30)
# Project name, version and description
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

project(mirror-down-detector
    VERSION 1.0
    DESCRIPTION "A discord bot"
    LANGUAGES CXX
)

include (cmake/fetch_deps.cmake)

add_subdirectory(src)

