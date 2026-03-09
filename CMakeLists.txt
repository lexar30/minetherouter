cmake_minimum_required(VERSION 4.3.0)

project(minetherouter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# common-lib
add_library(minetherouter-common-lib STATIC
	# src network 
	minetherouter-common/network/MessageHandler.cpp
	minetherouter-common/network/NetSession.cpp
	minetherouter-common/network/ProtocolContract.cpp
	
	# headers network / core
	minetherouter-common/network/MessageHandler.h
	minetherouter-common/network/NetSession.h
	minetherouter-common/network/ProtocolContract.h
	
	minetherouter-common/network/ByteReader.h
	minetherouter-common/network/ByteWriter.h
	minetherouter-common/network/Message.h
	minetherouter-common/network/MessageCodec.h
	minetherouter-common/network/MessageType.h
	
	# headers network / messages
	minetherouter-common/network/messages/JoinAccept.h
	minetherouter-common/network/messages/JoinReject.h
	minetherouter-common/network/messages/JoinRequest.h
	minetherouter-common/network/messages/Ping.h
	minetherouter-common/network/messages/Pong.h	
	
	# headers utils
	minetherouter-common/utils/Logger.h	
)

target_include_directories(minetherouter-common-lib
	PUBLIC
		minetherouter-common
)

add_executable(minetherouter-tests
	minetherouter-common/tests/TestMain.cpp
	minetherouter-common/tests/Tests.cpp
	minetherouter-common/tests/Tests.h
)

# tests
target_link_libraries(minetherouter-tests
	PRIVATE
		minetherouter-common-lib
)

add_executable(minetherouter-client
	minetherouter/main.cpp
)

# client
target_link_libraries(minetherouter-client
	PRIVATE
		minetherouter-common-lib
)

# server
add_executable(minetherouter-server
	minetherouter-server/main.cpp
)

target_link_libraries(minetherouter-server
	PRIVATE
		minetherouter-common-lib
)

