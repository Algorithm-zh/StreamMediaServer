#搜索源文件
aux_source_directory(. DIR_LIB_SRCS)
include_directories(.)
#生成静态库
add_library(base ${DIR_LIB_SRCS})
