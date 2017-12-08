project(libnetwork LANGUAGES CXX)

add_library(network
	src/Ipc.cpp
	#src/DataParser.cpp
	src/socket.cpp
	#src/UdpSocket.cpp
	src/TcpServerSocket.cpp
	src/ScanDriver.cpp
	#src/TcpClientSocket.cpp
	#src/TcpClient.cpp
	src/Network.cpp)

target_include_directories(network 
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE src)

target_link_libraries(network osal logging tools)
