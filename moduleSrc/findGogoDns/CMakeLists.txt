#set(FIND_GOGO_DNS_SRC_LIST findGogoDns.cpp)

#list(APPEND FIND_GOGO_DNS_SRC_LIST ${listSrc})
list(APPEND FIND_GOGO_DNS_SRC_LIST "findGogoDns.cpp")

#头文件目录
#message(WARNING "hello:moduleIncludes = [${moduleIncludes}]")
#message(WARNING "hello:srcList = [${srcList}]")
message(WARNING "findGOgoDns:FIND_GOGO_DNS_SRC_LIST = [${FIND_GOGO_DNS_SRC_LIST}]")
#include_directories (${moduleIncludes})

#库文件目录
message(WARNING "findGogoDns:CMAKE_LIBRARY_PATH = [${CMAKE_LIBRARY_PATH}]")

#可执行文件生成目录
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

ADD_EXECUTABLE(findGogoDns ${FIND_GOGO_DNS_SRC_LIST})

#target_link_libraries(lstLib)
TARGET_LINK_LIBRARIES(findGogoDns pthread)
TARGET_LINK_LIBRARIES(findGogoDns curl)
