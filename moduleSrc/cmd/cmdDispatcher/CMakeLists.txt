#set(CMD_DISPATCHER_SRC_LIST cmdDispatcher.cpp)

list(APPEND CMD_DISPATCHER_SRC_LIST ${listSrc})
list(APPEND CMD_DISPATCHER_SRC_LIST "cmdDispatcher.cpp")

#头文件目录
#message(WARNING "hello:moduleIncludes = [${moduleIncludes}]")
#message(WARNING "hello:srcList = [${srcList}]")
message(WARNING "cmdDispatcher:CMD_DISPATCHER_SRC_LIST = [${CMD_DISPATCHER_SRC_LIST}]")
#include_directories (${moduleIncludes})

#库文件目录

#可执行文件生成目录
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

ADD_EXECUTABLE(cmdDispatcher ${CMD_DISPATCHER_SRC_LIST})

#target_link_libraries(findGogoDns)
#TARGET_LINK_LIBRARIES(findGogoDns pthread)
#TARGET_LINK_LIBRARIES(findGogoDns curl)