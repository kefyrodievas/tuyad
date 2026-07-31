set( APP_NAME "tuyad" )

# Demo target.
add_executable(
    ${APP_NAME}
        "src/${APP_NAME}.c"
        "src/arguments.c"
        "src/deviceinfo.c"
        "src/actions.c"
        "src/daemon.c"
        "src/tuya_func.c"
        "src/memory.c"
)

target_link_libraries(
    ${APP_NAME}
    PUBLIC
        link_core
        middleware_implementation
        platform_port
        utils_modules
        blobmsg_json
        ubox
        ubus
)

target_include_directories(
    ${APP_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

install(TARGETS ${APP_NAME} DESTINATION /usr/bin)