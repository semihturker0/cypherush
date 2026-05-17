qt_add_resources(CYPHERUSH_RESOURCES resources/icons.qrc)

add_executable(cypherush_client
    main.cpp
    ChatClient.cpp
    services/AuthenticationService.cpp
    services/MessagingService.cpp
    ui/SplashScreen.cpp
    ui/LoginWindow.cpp
    ui/MainChatWindow.cpp
    ${CYPHERUSH_RESOURCES}
)

target_include_directories(cypherush_client
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(cypherush_client
    PRIVATE
        cypherush_common
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
)
