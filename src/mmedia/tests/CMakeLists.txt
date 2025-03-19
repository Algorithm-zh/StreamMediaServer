add_executable(HandShakeServerTest HandShakeServerTest.cpp)
target_link_libraries(HandShakeServerTest base network mmedia crypto)
install(TARGETS HandShakeServerTest RUNTIME DESTINATION sbin)

add_executable(HandShakeClientTest HandShakeClientTest.cpp)
target_link_libraries(HandShakeClientTest base network mmedia crypto)
install(TARGETS HandShakeClientTest RUNTIME DESTINATION sbin)

add_executable(RtmpServerTest RtmpServerTest.cpp)
target_link_libraries(RtmpServerTest base network mmedia crypto)
install(TARGETS RtmpServerTest RUNTIME DESTINATION sbin)
