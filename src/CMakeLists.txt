# Create an executable
add_executable(mirror-down-detector
    main.cpp
    readFile.cpp
    ping.cpp
    bot.cpp
    background.cpp
)

# Link the pre-installed DPP package.
target_link_libraries(mirror-down-detector 
PRIVATE    
    dpp
)

# Include the DPP directories.
target_include_directories(mirror-down-detector
PRIVATE
    ../include
)

# Set C++ version
set_target_properties(mirror-down-detector 
PROPERTIES
    CXX_STANDARD 23
)
