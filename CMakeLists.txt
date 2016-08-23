cmake_minimum_required(VERSION 2.8)

PROJECT (beta)

#暂时屏蔽
#set(CMAKE_C_FLAGS "-O2 -Wall -fsigned-char -Wno-unused")
#set(CMAKE_CXX_FLAGS "-O2 -Wall -fsigned-char -Wno-unused")

#set(CMAKE_C_FLAGS_RELEASE "-O2")
#set(CMAKE_CXX_FLAGS_RELEASE "-O2")
#set(CMAKE_C_FLAGS "-Wall")
#set(CMAKE_CXX_FLAGS "-Wall")

#设置项目关键路径
set(PROJECT_ROOT_DIRECTORY /home/leo/beta)
set(PROJECT_BINARY_DIR "${PROJECT_ROOT_DIRECTORY}/bin")
set(PROJECT_INCLUDE_DIR "${PROJECT_ROOT_DIRECTORY}/moduleInclude")
set(PROJECT_SOURCE_DIR "${PROJECT_ROOT_DIRECTORY}/moduleSrc")

#指定可执行文件的输出目录，输出到bin下面
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_ROOT_DIRECTORY}/bin)

#获取头文件目录 --- 需要一个递归获取子目录名的函数
file(GLOB moduleIncludes moduleInclude/*)

#framework目录下子文件夹
file(GLOB frameworkIncludes moduleInclude/framework/*)
list(APPEND moduleIncludes ${frameworkIncludes})
#message(WARNING "main:frameworkIncludes = [${frameworkIncludes}]")

#包含系统目录下的curl头文件
set(CURL_HEADFILE_FOLDER "/usr/include")
list(APPEND moduleIncludes ${CURL_HEADFILE_FOLDER})

message(WARNING "main:moduleIncludes = [${moduleIncludes}]")
include_directories (${moduleIncludes})

#message(WARNING "main:PROJECT_ROOT_DIRECTORY = [${PROJECT_ROOT_DIRECTORY}]")
#message(WARNING "main:PROJECT_BINARY_DIR = [${PROJECT_BINARY_DIR}]")
#message(WARNING "main:PROJECT_INCLUDE_DIR = [${PROJECT_INCLUDE_DIR}]")
#message(WARNING "main:PROJECT_SOURCE_DIR = [${PROJECT_SOURCE_DIR}]")


#获取源文件--需要自己添加子目录名 -- 尝试用file替代
#遍历所有cpp目录，形成SRC_LIST

aux_source_directory("${PROJECT_SOURCE_DIR}/framework/list" listSrc)
list(APPEND srcList ${listSrc})

aux_source_directory("${PROJECT_SOURCE_DIR}/debug" debugSrc)
list(APPEND srcList ${debugSrc})

aux_source_directory("${PROJECT_SOURCE_DIR}/findGogoDns" findGogoDnsSrc)
list(APPEND srcList ${findGogoDnsSrc})

aux_source_directory("${PROJECT_SOURCE_DIR}/cmd/cmdDispatcher" cmdDispatcherSrc)
list(APPEND srcList ${cmdDispatcherSrc})

aux_source_directory("${PROJECT_SOURCE_DIR}/main" mainSrc)
list(APPEND srcList ${mainSrc})

message(WARNING "main:srcList = [${srcList}]")

#add_subdirectory("${PROJECT_SOURCE_DIR}")

add_executable(main "${srcList}")

TARGET_LINK_LIBRARIES(main pthread)
TARGET_LINK_LIBRARIES(main stdc++)
TARGET_LINK_LIBRARIES(main rt)
TARGET_LINK_LIBRARIES(main curl)
