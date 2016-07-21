#头文件目录
#message(WARNING "hello:moduleIncludes = [${moduleIncludes}]")
#message(WARNING "hello:srcList = [${srcList}]")
#message(WARNING "hello:HELLO_SRC_LIST = [${HELLO_SRC_LIST}]")
#include_directories (${moduleIncludes})

#库文件目录

#可执行文件生成目录
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

#ADD_EXECUTABLE(hello ${HELLO_SRC_LIST})

#add_subdirectory("${PROJECT_SOURCE_DIR}/debug")
#add_subdirectory("${PROJECT_SOURCE_DIR}/findGogoDns")