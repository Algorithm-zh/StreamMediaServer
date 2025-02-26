#搜索源文件
aux_source_directory(. DIR_LIB_SRCS)

include_directories(${PROJECT_SOURCE_DIR}/lib/jsoncpp/include)
link_directories(${PROJECT_SOURCE_DIR}/lib/jsoncpp/lib)
link_libraries(jsoncpp.a)

#设置静态库生成路径
set(LIBRARY_OUTPUT_PATH ${LIB_PATH})
#生成静态库
add_library(base ${DIR_LIB_SRCS})
target_link_libraries(base jsoncpp.a)
