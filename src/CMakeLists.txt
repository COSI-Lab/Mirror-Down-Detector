add_executable(down-detector)

target_sources(down-detector
PRIVATE
    main.cpp
    readFile.cpp
    ping.cpp
    bot.cpp
    background.cpp
    http.cpp
)

target_include_directories(down-detector
PRIVATE
    ../include
)

target_link_libraries(down-detector
PRIVATE
    curl
    dpp
    spdlog::spdlog
)

set_target_properties(down-detector 
PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)
