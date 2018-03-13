project(libnetwork LANGUAGES CXX)

add_library(network
	src/Ipc.cpp
	#src/DataParser.cpp
	src/socket.cpp
	#src/UdpSocket.cpp
	src/TcpServerSocket.cpp
	src/ScanDriver.cpp
	src/NetworkConfig.cpp
	src/FdManager.cpp
	#src/TcpClientSocket.cpp
	#src/TcpClient.cpp
	src/TcpServer.cpp
	src/MyServer.cpp
	src/Buffer.cpp
	src/Acceptor.cpp
	src/TcpConnection.cpp
	src/SocketOps.cpp
	src/InetAddress.cpp
	src/EventLoop.cpp
	src/Channel.cpp
	src/Poller.cpp
	src/Network.cpp)

target_include_directories(network 
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE src)

target_link_libraries(network osal logging tools)
