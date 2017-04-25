set(LIB_HELLO_SRC hello.c)

add_library(libTestHello ${LIB_HELLO_SRC})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set_target_properties(libTestHello PROPERTIES OUTPUT_NAME "hello")
