#搜索源文件
aux_source_directory(. DIR_LIB_SRCS)
#设置静态库生成路径
set(LIBRARY_OUTPUT_PATH ${LIB_PATH})
#生成静态库
add_library(base ${DIR_LIB_SRCS})
