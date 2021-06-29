CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#project name
PROJECT(test0)

#添加源文件
aux_source_directory(./ DIR_SRCS)
aux_source_directory(./module1 DIR_SRCS)
aux_source_directory(./module2 DIR_SRCS)
aux_source_directory(./module3 DIR_SRCS)


#添加头文件路径
include_directories( 
	./
	./module1
	./module2
	./module3
    ../include
)


find_library(FASTLOG fastlog ../build/)
find_library(PTHREAD pthread /usr/lib64)

link_libraries(${FASTLOG} ${PTHREAD})

add_definitions( -MD -Wall  -g -ggdb -DFASTLOG_HAVE_LIBXML2)

#generate application test0 
ADD_EXECUTABLE(test0 ${DIR_SRCS})

