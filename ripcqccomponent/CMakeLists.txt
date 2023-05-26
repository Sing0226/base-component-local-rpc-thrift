cmake_minimum_required(VERSION 3.15)
project(ipcqcomponent)

set(PROJECT_NAME ipcqcomponent)
set(PROJECT_NAME_STATITC ipcqcomponent_static)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


set(REQUIRED_QT_VERSION 5.1.0)

option(ENABLE_QT5 "Enable Qt5" On)
option(ENABLE_QT6 "Enable Qt6 im module" Off)
option(ENABLE_LIBRARY "Qt library" On)

add_definitions(-DWRIPC_QC_LIB)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_definitions(-DHAVE_INTTYPES_H)
	add_definitions(-DHAVE_NETINET_IN_H)
endif()


MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE(STATUS "current platform: Linux ")
    SET(LINUX 1)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    MESSAGE(STATUS "current platform: Windows")
    SET(OS_WIN 1)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    MESSAGE(STATUS "current platform: FreeBSD")
ELSE ()
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

MESSAGE(STSTUS "###################################")

if (OS_WIN)
	set(THRIFT_COMPILER ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/bin/thrift.exe)
else()
        set(THRIFT_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/bin/thrift")
endif()
set(THRIFT_INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/ripcqcservices.thrift)

set(ripcqc_services_INC ${CMAKE_CURRENT_BINARY_DIR}/ripcqcservices)

set(ripcqc_services_SRC
	${ripcqc_services_INC}/gen-cpp/ripcqcservices.cpp
	${ripcqc_services_INC}/gen-cpp/ripcqcservices.h
	${ripcqc_services_INC}/gen-cpp/ripcqcservices_constants.cpp
	#${ripcqc_services_INC}/gen-cpp/ripcqcservices_async_server.skeleton.cpp
	${ripcqc_services_INC}/gen-cpp/ripcqcservices_constants.h
	#${ripcqc_services_INC}/gen-cpp/ripcqcservices_async_server.skeleton.h
	${ripcqc_services_INC}/gen-cpp/ripcqcservices_types.cpp
	${ripcqc_services_INC}/gen-cpp/ripcqcservices_types.h
	)

if(NOT EXISTS ${ripcqc_services_INC})
	file(MAKE_DIRECTORY ${ripcqc_services_INC})
endif()
# MESSAGE("thrift complier path ${THRIFT_COMPILER}")
# MESSAGE("-o ${ripcqc_services_INC}")
# MESSAGE("-input ${THRIFT_INPUT_FILE}")

#.\thrift.exe -o F:/workspace/rayshape/atqc/build/rsprometheusipc/ripcqcservices -strict -r	--gen cpp:moveable_types,no_default_operators,cob_style,no_client_completion "F:/workspace/rayshape/atqc/rsprometheusipc/ripcqcservices.thrift"
ADD_CUSTOM_COMMAND(
	OUTPUT ${ripcqc_services_SRC}
	COMMAND ${THRIFT_COMPILER} -o "${ripcqc_services_INC}" -strict -r 
		--gen cpp:moveable_types,no_default_operators,cob_style,no_client_completion "${THRIFT_INPUT_FILE}"
	MAIN_DEPENDENCY "${THRIFT_INPUT_FILE}"
	DEPENDS "${THRIFT_INPUT_FILE}"
	COMMENT "Generating source code from Thrift definition"
	)

	
if (OS_WIN)
	set(QT_PATH D:\\Qt\\5.15.2\\msvc2019_64\\lib)
else()
	set(QT_PATH /opt/Qt/5.15.2/gcc_64)
endif()

set(CMAKE_PREFIX_PATH ${QT_PATH})
message("-- The qt path:${CMAKE_PREFIX_PATH}")
if (OS_WIN)
	set(Qt5_DIR "${CMAKE_PREFIX_PATH}\\cmake\\Qt5")
else()
	set(Qt5_DIR "${CMAKE_PREFIX_PATH}/cmake/Qt5")
endif()
message("The qt5 dir:${Qt5_DIR}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ipcbase)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/boost_1_81_0)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/boost_1_81_0/boost)
include_directories(${ripcqc_services_INC})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../logger)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if (OS_WIN)
	set(lib_thrift_time_SRC  
		3rdparty/thrift/windows/GetTimeOfDay.cpp
		3rdparty/thrift/windows/GetTimeOfDay.h
	)
endif()

set(lib_src 
		stdafx.h
		stdafx.cpp
		global.h
		typedef.h
		../logger/rslogger_declare.h
		../logger/rslogging.h
		../logger/rslog.h
		ipcbase/ripc.h
		ipcbase/ripcclientchannel.cpp
		ipcbase/ripcclientchannel.h
		ipcbase/ripcconstant.h
		ipcbase/ripcservertoclientchannel.cpp
		ipcbase/ripcservertoclientchannel.h
		ipcbase/ripcservertoclientchannelprivate.cpp
		ipcbase/ripcservertoclientchannelprivate.h
		ipcbase/rlocalserver.cpp
		ipcbase/rlocalserver.h
		ipcbase/rlocalsocketdef.h

		ripcqcserviceshost.cpp
		ripcqcserviceshost.h
		ripcqcserviceshostprivate.cpp
		ripcqcserviceshostprivate.h
		ripcqcclient.h
		ripcqcclient.cpp

		include/ipc_error.h
		include/ipc_declare.h
		include/ipc_client_interface.h

		ripcqcservice.h
		ripcqcservicescobclient.h
		ripcqcservicescobclient.cpp
		ripcqchandlercob.h
		ripcqcservicesqcdataproxy.h
		ripcqcservicesqcdataproxy.cpp
		ripcqcservicesdataproxy.h
		ripcqcservicesdataproxy.cpp
		# ripcqcclientinterface.h
		ripcqcclientinterface.cpp
		ripcqcinterfaceproxy.h
		ripcqcinterfaceproxy.cpp
		ripcqcinterfaceimpl.h
		ripcqcinterfaceimpl.cpp
		ripcsingleton.h
		abstractrequestrunqctask.h
		ripcqcservicesnotifytasks.h
		ripcqcservicesnotifytasks.cpp
		
		#thrift
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/TApplicationException.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/TApplicationException.h
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/TOutput.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/TOutput.h
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/concurrency/Mutex.h
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/concurrency/Mutex.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/concurrency/Monitor.h
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/concurrency/Monitor.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/async/TAsyncChannel.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/async/TAsyncChannel.h
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/async/TAsyncProtocolProcessor.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/async/TAsyncProtocolProcessor.h
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/async/TConcurrentClientSyncInfo.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/async/TConcurrentClientSyncInfo.h
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/protocol/TProtocol.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/protocol/TProtocol.h
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/transport/TBufferTransports.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/transport/TBufferTransports.h
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/transport/TTransportException.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/transport/TTransportException.h
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/transport/TTransportUtils.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/thrift/transport/TTransportUtils.h
		${lib_thrift_time_SRC}
		${ripcqc_services_SRC}
        )

if (OS_WIN)
	set(lib_socket_src  
		ipcbase/rlocalsocket.h
		ipcbase/rlocalsocket.cpp
		)
else()
	set(lib_socket_src  
                ipcbase/rqtlocalsocket.h
                ipcbase/rqtlocalsocket.cpp
                ipcbase/rmsglocalserver_unix.cpp
                ipcbase/rmsglocalserver_unix.h
		)
endif()

# find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)
# find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core Widgets Network)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network)

if (OS_WIN)
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
add_library(${PROJECT_NAME_STATITC} STATIC ${lib_src} ${lib_socket_src})
set_target_properties(${PROJECT_NAME_STATITC} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(${PROJECT_NAME_STATITC} Qt5::Core Qt5::Widgets Qt5::Network logger)

add_library(${PROJECT_NAME} SHARED ${lib_src} ${lib_socket_src})
#set_target_properties(${PROJECT_NAME} PROPERTIES VERSION 1.0 SOVERSION 1)
#set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
#set_target_properties(${PROJECT_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
# Link libraries
# qt5_use_modules(${PROJECT_NAME} Core Gui Network)
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets Qt5::Network logger)
target_link_libraries(${PROJECT_NAME})
else()
add_compile_options(-std=c++17 -pthread -g)


add_library(${PROJECT_NAME_STATITC} STATIC ${lib_src} ${lib_socket_src})
# 将 rspromethusipc_static 重命名为 rspromethusipc
set_target_properties(${PROJECT_NAME_STATITC} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
# cmake 在构建一个新的 target 时，会尝试清理掉使用这个名称的库，
# 所以在构建 libadd.so 时，就会清理掉 libadd.a
set_target_properties(${PROJECT_NAME_STATITC} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(${PROJECT_NAME_STATITC} Qt5::Core Qt5::Widgets Qt5::Network logger)

add_library(${PROJECT_NAME} SHARED ${lib_src} ${lib_socket_src})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION 1.0 SOVERSION 1)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets Qt5::Network logger)

endif()

