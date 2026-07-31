set( APP_NAME "tuyad" )

# Demo target.
add_executable(
    ${APP_NAME}
        "${APP_NAME}.c"
        "arguments.c"
        "deviceinfo.c"
        "actions.c"
        "daemon.c"
        "tuya_func.c"
)

target_link_libraries(
    ${APP_NAME}
    PUBLIC
        tuya_core
)

target_include_directories(
    ${APP_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)
