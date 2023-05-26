cmake_minimum_required(VERSION 3.14)

project(rpc_srv_demo_linux LANGUAGES CXX)
set(PROJECT_NAME rpc_srv_demo_linux)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(REQUIRED_QT_VERSION 5.1.0)
option(ENABLE_QT5 "Enable Qt5" On)
option(ENABLE_QT6 "Enable Qt6 im module" Off)
option(ENABLE_LIBRARY "Qt library" On)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(Qt5_DIR "D:\\Qt\\5.15.2\\msvc2019_64\\lib\\cmake\\Qt5")
else()
    set(QT_PATH /opt/Qt/5.15.2/gcc_64)
    set(CMAKE_PREFIX_PATH ${QT_PATH})
    message("-- The qt path:${CMAKE_PREFIX_PATH}")

    set(Qt5_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Qt5")
    message("The qt5 dir:${Qt5_DIR}")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_compile_options(-std=c++17 -pthread -g -Wall -O0 -fstack-protector -fstack-protector-all)
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_definitions(-DHAVE_INTTYPES_H)
	add_definitions(-DHAVE_NETINET_IN_H)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../logger)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ripcqccomponent)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ripcqccomponent/3rdparty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ripcqccomponent/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ripcqccomponent/ipcbase)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ripcqccomponent/3rdparty/boost_1_81_0)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../ripcqccomponent/3rdparty/boost_1_81_0/boost)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../logger)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/threadpool)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB THREADPOOL_DIR_SRC threadpool/*.cpp threadpool/*.h)
source_group("ThreadPool" FILES ${THREADPOOL_DIR_SRC})


add_executable(${PROJECT_NAME}
    ../ripcqccomponent/include/ipc_declare.h
    ../ripcqccomponent/include/ipc_error.h
    ../ripcqccomponent/ripcqcservice.h
    ../ripcqccomponent/ripcqcserviceshost.h
    ../ripcqccomponent/ipcbase/ripc.h
    ../ripcqccomponent/global.h
    ../ripcqccomponent/abstractrequestrunqctask.h
    ../logger/rslogger_declare.h
    ../logger/rslogging.h
    ../logger/rslog.h
    ${THREADPOOL_DIR_SRC}
    src/ripcqcserver.h
    src/ripcqcserver.cpp
    src/requestrunqctask.h
    src/requestrunqctask.cpp
    src/authorizedinfojob.h
    src/authorizedinfojob.cpp
    src/userloginjob.h
    src/userloginjob.cpp
    src/rsingleton.h
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME} 
	Qt${QT_VERSION_MAJOR}::Core 
	Qt${QT_VERSION_MAJOR}::Widgets 
	Qt${QT_VERSION_MAJOR}::Network 
    logger
    pthread
    ipcqcomponent
    )
