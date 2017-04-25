project(APP)

#开启详细编译提示信息
set(CMAKE_VERBOSE_MAKEFILE ON)

#开启src目录编译
add_subdirectory(src)

link_directories(lib)

#开启hello库编译
#add_subdirectory(libHello)
