CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#project name
PROJECT(fdecoder)

#添加源文件
AUX_SOURCE_DIRECTORY(. SRC_LIST)

aux_source_directory(./ DIR_SRCS)
aux_source_directory(bitmask DIR_SRCS)
aux_source_directory(hiredis DIR_SRCS)
aux_source_directory(linenoise DIR_SRCS)


#添加头文件路径
include_directories( 
	./
    ./bitmask
    ./hiredis
    ./linenoise
    ../include
    /usr/include/libxml2
)


find_library(XML2 xml2 /usr/lib64)
find_library(JSON json-c /usr/lib64)
find_library(FASTLOG fastlog ../build/)

link_libraries(${XML2} ${JSON} ${FASTLOG})

add_definitions( -MD -Wall  -g -ggdb -DFASTLOG_HAVE_LIBXML2 -DFASTLOG_HAVE_JSON)

#generate application hello 
ADD_EXECUTABLE(fdecoder ${SRC_LIST} ${DIR_SRCS})
