# Minimum CMake version required, we'll just use the latest version.
cmake_minimum_required(VERSION 3.22)
# Project name, version and description
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")


project(discord-bot VERSION 1.0 DESCRIPTION "A discord bot")

# Create an executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/readFile.cpp
    src/ping.cpp
    src/bot.cpp
    src/background.cpp
    src/http.cpp
)
 
# Find our pre-installed DPP package (using FindDPP.cmake).
find_package(DPP REQUIRED)
 
# Link the pre-installed DPP package.
target_link_libraries(${PROJECT_NAME} 
    curl
    libdpp.so
)

# Set C++ version
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)