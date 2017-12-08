project(libtools LANGUAGES CXX)

add_library(tools
	src/bmp.cpp)
	#src/timer.cpp
	#src/test.cpp)
	
target_include_directories(tools 
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE src)

target_link_libraries(tools osal logging)
