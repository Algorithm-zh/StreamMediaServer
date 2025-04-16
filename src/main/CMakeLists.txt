aux_source_directory(. SRCS)
add_executable(tmms ${SRCS})

add_dependencies(base jsoncpp)
target_link_libraries(tmms base network mmedia live jsoncpp_static.a crypto)
install(TARGETS tmms RUNTIME DESTINATION sbin)