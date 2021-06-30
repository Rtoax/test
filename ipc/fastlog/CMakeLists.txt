CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(fastlog)

#添加源文件
aux_source_directory(./src DIR_SRCS)

#设置编译参数 
set(CMAKE_C_FLAGS "-g -Wall")

#添加头文件路径
include_directories( 
    ./include
    ./src
    /usr/include/libxml2
)
    
#添加库
find_library(RT rt /usr/lib64) 
find_library(DL dl /usr/lib64) 
find_library(MATH m /usr/lib64)
find_library(XML2 m /usr/lib64)

#link库
link_libraries(${DL} ${RT} ${MATH})

#定义
add_definitions(-g3 -MD -Wall -pthread -g -ggdb -O3 -D_FASTLOG_USE_EVENTFD=1)

#生成动态库和静态库 
add_library(acc_lib_shared  SHARED ${DIR_SRCS}) 
add_library(acc_lib_static 	STATIC ${DIR_SRCS})

#将动态库和静态库的名字设置
set_target_properties(acc_lib_shared PROPERTIES OUTPUT_NAME "fastlog") 
set_target_properties(acc_lib_static PROPERTIES OUTPUT_NAME "fastlog")

#设置动态库版本 
set_target_properties(acc_lib_shared PROPERTIES VERSION 1.0 SOVERSION 1)