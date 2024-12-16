---------------------------------------------------
# This file is generated by the Qt Visual Studio Tools.
# ------------------------------------------------------

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle

TARGET = DonTcpServer

DESTDIR = ../bin
OBJECTS_DIR = ../tmp/DonTcpServer

QMAKE_CXXFLAGS +=  -Wno-unused-parameter

# DEFINES += DON_SOCKET_SELECT

HEADERS += server.h \
    server_socket_callback.h \
    server_command_callback.h \
    cmd_stat.h \
    cmd_broadcast.h \
    ../DonSocket/Buffer.h \
    ../DonSocket/DonTcpClient.h \
    ../DonSocket/DonTcpServer.h \
    ../DonSocket/Iocp.h \
    ../DonSocket/MemPool.h \
    ../DonSocket/Message.h \
    ../DonSocket/Select.h \
    ../DonSocket/Epoll.h \
    ../DonSocket/Kqueue.h \
    ../DonSocket/Socket.h \
    ../DonSocket/Config.h \
    ../Util/Util.h \
    ../Util/Log.h \
    ../Util/Console.h \
    ../CommandEngine/CommandEngine.h \
    ../CommandEngine/CommandObject.h \
	../CommandEngine/Executer.h \
	../CommandEngine/ExecuterThread.h


SOURCES += main.cpp \
    server.cpp \
    server_socket_callback.cpp \
    server_command_callback.cpp \
    cmd_stat.cpp \
    cmd_broadcast.cpp \
    ../DonSocket/Buffer.cpp \
    ../DonSocket/DonTcpClient.cpp \
    ../DonSocket/DonTcpServer.cpp \
    ../DonSocket/Iocp.cpp \
    ../DonSocket/Select.cpp \
    ../DonSocket/Epoll.cpp \
    ../DonSocket/Kqueue.cpp \
    ../DonSocket/Socket.cpp \
    ../Util/Util.cpp \
    ../Util/Log.cpp \
    ../Util/Console.cpp \
	../CommandEngine/CommandEngine.cpp \
    ../CommandEngine/CommandObject.cpp \
    ../CommandEngine/Executer.cpp \
	../CommandEngine/ExecuterThread.cpp


LIBS += -lpthread