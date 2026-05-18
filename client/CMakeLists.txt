# Cypherush - Copyright (c) 2026 semihturker0. All Rights Reserved.
# Proprietary software. See LICENSE for terms.
qt_add_resources(CYPHERUSH_RESOURCES resources/icons.qrc)

add_executable(cypherush_client
    main.cpp
    ChatClient.cpp
    services/AuthenticationService.cpp
    services/MessagingService.cpp
    services/ServerConfig.cpp
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

if(WIN32)
    target_link_libraries(cypherush_client PRIVATE dwmapi)
endif()
