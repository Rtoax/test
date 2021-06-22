CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#project name
PROJECT(test0)

#添加源文件
AUX_SOURCE_DIRECTORY(. SRC_LIST)

aux_source_directory(./ DIR_SRCS)


#添加头文件路径
include_directories( 
	./
    ../include
)


find_library(FASTLOG fastlog ../build/)
find_library(PTHREAD pthread /usr/lib64)

link_libraries(${FASTLOG} ${PTHREAD})

add_definitions( -MD -Wall  -g -ggdb -DFASTLOG_HAVE_LIBXML2)

#generate application test0 
ADD_EXECUTABLE(test0 ${SRC_LIST} ${DIR_SRCS})

