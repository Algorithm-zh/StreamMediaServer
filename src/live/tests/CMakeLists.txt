add_executable(CodecHeaderTest CodecHeaderTest.cpp)
target_link_libraries(CodecHeaderTest base network mmedia live crypto)
install(TARGETS CodecHeaderTest RUNTIME DESTINATION sbin)

