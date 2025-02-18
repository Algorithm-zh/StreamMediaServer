aux_source_directory(. SRCS)
add_executable(tmms ${SRCS})
install(TARGETS tmms RUNTIME DESTINATION sbin)
