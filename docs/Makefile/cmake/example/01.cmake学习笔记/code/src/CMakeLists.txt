include_directories(${PROJECT_SOURCE_DIR}/libHello)

set(APP_SRC_LIST main.c)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

link_directories("../lib")

add_executable(app ${APP_SRC_LIST})

target_link_libraries(app libTestHello)
