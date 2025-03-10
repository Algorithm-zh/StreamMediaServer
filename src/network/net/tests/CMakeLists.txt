add_executable(EventLoopThreadTest EventLoopThreadTest.cpp)
target_link_libraries(EventLoopThreadTest network)
install(TARGETS EventLoopThreadTest RUNTIME DESTINATION sbin)

add_executable(InetAddressTest InetAddressTest.cpp)
target_link_libraries(InetAddressTest base network)
install(TARGETS InetAddressTest RUNTIME DESTINATION sbin)

add_executable(SocketOptTest SocketOptTest.cpp)
target_link_libraries(SocketOptTest base network)
install(TARGETS SocketOptTest RUNTIME DESTINATION sbin)

add_executable(AcceptorTest AcceptorTest.cpp)
target_link_libraries(AcceptorTest base network)
install(TARGETS AcceptorTest RUNTIME DESTINATION sbin)
