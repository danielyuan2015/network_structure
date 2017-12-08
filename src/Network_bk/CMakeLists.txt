project(libnetwork LANGUAGES CXX)

add_library(network
	src/socket.cpp)
	#src/UdpSocket.cpp
	#src/TcpServerSocket.cpp
	#src/TcpClientSocket.cpp
	#src/TcpClient.cpp
	#src/TcpServer.cpp)

target_include_directories(network 
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE src)
target_link_libraries(network logging)
#target_link_libraries(network tools product scandriver osal logging)
