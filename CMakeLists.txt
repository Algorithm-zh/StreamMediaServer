
cmake_minimum_required(VERSION 2.6)
project(tmms)

# 设置安装路径
set(CMAKE_INSTALL_PREFIX ../bin)

# 设置构建类型
set(CMAKE_BUILD_TYPE Debug)

# 设置编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Building in Debug mode")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -g -pipe -Wall -Werror -Wno-unused-variable -lrt -pthread")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Building in Release mode")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -g -pipe -Wall -Werror -Wno-unused-variable -lrt -pthread")
endif()

# 设置通用编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing -Wno-error=shadow -Wno-sign-compare")

# 打印 GCC 版本
message(STATUS "USING GCC VERSION: ${CMAKE_CXX_COMPILER_VERSION}")

# 处理 GCC 版本检查
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "7.2.9")
  message(STATUS "GCC Version > 7.2.9, adding -Wno-error=unused-result and -Werror=return-type")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=unused-result -Werror=return-type")
endif()

# 设置静态库生成路径和名称
set(LIB_PATH ${PROJECT_SOURCE_DIR}/lib)
set(BASE_LIB base)

# 包含源代码目录
include_directories(src)

# 添加子目录
add_subdirectory(src)
