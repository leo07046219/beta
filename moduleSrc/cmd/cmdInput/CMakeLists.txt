cmake_minimum_required(VERSION 2.8)

PROJECT (inputCmd)

set(CMAKE_C_FLAGS ”-O2 -Wall -fsigned-char -Wno-unused”)

#设置项目关键路径
set(PROJECT_ROOT_DIRECTORY /home/leo/beta/moduleSrc/cmd/cmdInput)
set(PROJECT_BINARY_DIR "${PROJECT_ROOT_DIRECTORY}/bin")
set(PROJECT_INCLUDE_DIR "${PROJECT_ROOT_DIRECTORY}/moduleInclude")
set(PROJECT_SOURCE_DIR "${PROJECT_ROOT_DIRECTORY}/moduleSrc")

#指定可执行文件的输出目录，输出到bin下面
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_ROOT_DIRECTORY}/bin)

#获取头文件目录 --- 需要一个递归获取子目录名的函数
set(CMD_INPUT_HEADFILE_FOLDER "${PROJECT_ROOT_DIRECTORY}")
list(APPEND moduleIncludes ${CMD_INPUT_HEADFILE_FOLDER})
message(WARNING "main:moduleIncludes = [${moduleIncludes}]")
include_directories (${moduleIncludes})

message(WARNING "main:PROJECT_ROOT_DIRECTORY = [${PROJECT_ROOT_DIRECTORY}]")
message(WARNING "main:PROJECT_BINARY_DIR = [${PROJECT_BINARY_DIR}]")


set(inputCmdSrc "${PROJECT_ROOT_DIRECTORY}/cmdInput.cpp")

message(WARNING "inputCmd:inputCmdSrc = [${inputCmdSrc}]")

add_executable(inputCmd "${inputCmdSrc}")

TARGET_LINK_LIBRARIES(inputCmd stdc++)
