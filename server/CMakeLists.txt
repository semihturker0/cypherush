add_executable(cypherush_server
    main.cpp
    ChatServer.cpp
)

target_include_directories(cypherush_server
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cypherush_server
    PRIVATE
        cypherush_common
        Qt6::Core
        Qt6::Network
)
