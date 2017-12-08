project(app_barcode_scanner)
set(TARGET_NAME "tcp_main")

add_executable(
	${TARGET_NAME}
	main.cpp
	)
	
target_link_libraries(${TARGET_NAME}
	#product
	#logging
	network
	pthread
)

get_filename_component(TOOLCHAIN_PATH "${CMAKE_C_COMPILER}" PATH)
#add_custom_command(
#	TARGET ${TARGET_NAME}
#	POST_BUILD
    #COMMAND mkdir -p ${CMAKE_BINARY_DIR}/stripped
	#COMMAND ${TOOLCHAIN_PATH}/arm-poky-linux-gnueabi-strip -s ${CMAKE_BINARY_DIR}/${TARGET_NAME} -o ${CMAKE_BINARY_DIR}/stripped/${TARGET_NAME}
	#COMMAND ${TOOLCHAIN_PATH}/arm-poky-linux-gnueabi-objcopy --only-keep-debug ${CMAKE_BINARY_DIR}/${TARGET_NAME} ${CMAKE_BINARY_DIR}/stripped/${TARGET_NAME}.debug
	#COMMAND ${TOOLCHAIN_PATH}/arm-poky-linux-gnueabi-objcopy --add-gnu-debuglink ${CMAKE_BINARY_DIR}/stripped/${TARGET_NAME}.debug ${CMAKE_BINARY_DIR}/stripped/${TARGET_NAME}
#)
