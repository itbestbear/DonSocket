# https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html

cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_BUILD_TYPE "DEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -s" )
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -s" )


#MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")

#[[
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_compile_definitions(DON_SOCKET_EPOLL)
elseif (CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_compile_definitions(DON_SOCKET_IOCP)
elseif (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_compile_definitions(DON_SOCKET_KQUEUE)
else()
	MESSAGE(STATUS "unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()
]]

set(APP_SRC 
	server.h
	server.cpp
	server_socket_callback.h
	server_socket_callback.cpp
	server_command_callback.h
	server_command_callback.cpp
	cmd_stat.h
    cmd_stat.cpp
	cmd_broadcast.h
    cmd_broadcast.cpp
	main.cpp
	../DonSocket/Buffer.h
	../DonSocket/Buffer.cpp
	../DonSocket/DonTcpClient.cpp
	../DonSocket/DonTcpClient.h
	../DonSocket/DonTcpServer.cpp
	../DonSocket/DonTcpServer.h
	../DonSocket/MemPool.h
	../DonSocket/Message.h
	../DonSocket/Select.cpp
	../DonSocket/Select.h
	../DonSocket/Socket.cpp
	../DonSocket/Socket.h
	../DonSocket/Config.h
	../CommandEngine/CommandEngine.h
	../CommandEngine/CommandEngine.cpp
	../CommandEngine/CommandObject.h
	../CommandEngine/CommandObject.cpp
	../CommandEngine/Executer.h
	../CommandEngine/Executer.cpp
	../CommandEngine/ExecuterThread.h
	../CommandEngine/ExecuterThread.cpp
	../Util/Log.cpp
	../Util/Log.h
	../Util/Util.cpp
	../Util/Util.h
	../Util/Console.cpp
	../Util/Console.h
)	#APP_SRC变量，定义APP的源码文件列表

set(WIN_SRC 
	../DonSocket/Iocp.cpp
	../DonSocket/Iocp.h
)	#WIN_SRC变量，定义APP的源码文件列表

set(LIN_SRC 
	../DonSocket/Epoll.cpp
	../DonSocket/Epoll.h
)	#LIN_SRC变量，定义APP的源码文件列表

set(UNIX_SRC 
	../DonSocket/Kqueue.cpp
	../DonSocket/Kqueue.h
)	#UNIX_SRC变量，定义APP的源码文件列表

if(CMAKE_HOST_WIN32)
	message("Building platform: Windows ")

	set(EXECUTABLE_OUTPUT_PATH ../bin) #设置输出路径

	LINK_LIBRARIES(wsock32.lib ws2_32.lib ole32.lib)

	add_executable(DonTcpServer ${APP_SRC} ${WIN_SRC}) #根据APP_SRC生成执行文件

	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) #设置输出路径

elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
	message("Building platform: Linux ")

	set(EXECUTABLE_OUTPUT_PATH ../bin) #设置输出路径

	add_executable(DonTcpServer ${APP_SRC} ${LIN_SRC}) #根据APP_SRC生成执行文件

	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) #设置输出路径

	target_link_libraries(DonTcpServer pthread) #链接lib库

else()
	message("Building platform: Unix ")

	set(EXECUTABLE_OUTPUT_PATH ../bin) #设置输出路径

	add_executable(DonTcpServer ${APP_SRC} ${UNIX_SRC}) #根据APP_SRC生成执行文件

	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) #设置输出路径

	target_link_libraries(DonTcpServer pthread) #链接lib库

endif()
