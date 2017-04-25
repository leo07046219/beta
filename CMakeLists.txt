cmake_minimum_required(VERSION 2.8)

PROJECT (beta)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_COMPILER "/home/leo/toolChains/raspbian-toolchain-gcc-4.7.2-linux64/bin/arm-linux-gnueabihf-cc")
set(CMAKE_CXX_COMPILER "/home/leo/toolChains/raspbian-toolchain-gcc-4.7.2-linux64/bin/arm-linux-gnueabihf-c++")

#ÔÝÊ±ÆÁ±Î
#set(CMAKE_C_FLAGS "-O2 -Wall -fsigned-char -Wno-unused")
#set(CMAKE_CXX_FLAGS "-O2 -Wall -fsigned-char -Wno-unused")

#set(CMAKE_C_FLAGS_RELEASE "-O2")
#set(CMAKE_CXX_FLAGS_RELEASE "-O2")
#set(CMAKE_C_FLAGS "-Wall")
#set(CMAKE_CXX_FLAGS "-Wall")

#指定编译项目相关路径--迁移平台，文件夹发生变化时需要调整！
set(PROJECT_ROOT_DIRECTORY /home/leo/beta/beta)
set(PROJECT_BINARY_DIR "${PROJECT_ROOT_DIRECTORY}/bin")
set(PROJECT_INCLUDE_DIR "${PROJECT_ROOT_DIRECTORY}/moduleInclude")
set(PROJECT_SOURCE_DIR "${PROJECT_ROOT_DIRECTORY}/moduleSrc")

#指定生成的可执行文件路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_ROOT_DIRECTORY}/bin)

#添加moduleInclude目录下头文件
file(GLOB moduleIncludes moduleInclude/*)

#添加framework目录下头文件
file(GLOB frameworkIncludes moduleInclude/framework/*)
list(APPEND moduleIncludes ${frameworkIncludes})
#message(WARNING "main:frameworkIncludes = [${frameworkIncludes}]")

#libcurl头文件及库文件目录
set(CURL_HEADFILE_FOLDER "/home/leo/beta/beta/moduleSrc/framework/curl/curl-7.54.0/output/include/curl")
list(APPEND moduleIncludes ${CURL_HEADFILE_FOLDER})
set(CURL_LIB_ROUTINE "/home/leo/beta/beta/moduleSrc/framwork/curl/curl-7.54.0/output/lib")

message(WARNING "main:CURL_LIB_ROUTINE = [${CURL_LIB_ROUTINE}]")
message(WARNING "main:moduleIncludes = [${moduleIncludes}]")
include_directories (${moduleIncludes})

list(APPEND CMAKE_LIBRARY_PATH ${CURL_LIB_ROUTINE})
message(WARNING "main:CMAKE_LIBRARY_PATH = [${CMAKE_LIBRARY_PATH}]")


message(WARNING "main:PROJECT_ROOT_DIRECTORY = [${PROJECT_ROOT_DIRECTORY}]")
message(WARNING "main:PROJECT_BINARY_DIR = [${PROJECT_BINARY_DIR}]")
message(WARNING "main:PROJECT_INCLUDE_DIR = [${PROJECT_INCLUDE_DIR}]")
message(WARNING "main:PROJECT_SOURCE_DIR = [${PROJECT_SOURCE_DIR}]")


#link_directories("/home/leo/beta/beta/moduleSrc/framwork/curl/curl-7.54.0/output/lib")
#»ñÈ¡Ô´ÎÄ¼þ--ÐèÒª×Ô¼ºÌí¼Ó×ÓÄ¿Â¼Ãû -- ³¢ÊÔÓÃfileÌæ´ú
#±éÀúËùÓÐcppÄ¿Â¼£¬ÐÎ³ÉSRC_LIST

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

#link_libraries("/home/leo/beta/beta/moduleSrc/framwork/curl/curl-7.54.0/output/lib")
link_directories("/home/leo/beta/beta/moduleSrc/framework/curl/curl-7.54.0/output/lib")

add_executable(main "${srcList}")

TARGET_LINK_LIBRARIES(main pthread)
TARGET_LINK_LIBRARIES(main stdc++)
TARGET_LINK_LIBRARIES(main rt)
TARGET_LINK_LIBRARIES(main curl)
