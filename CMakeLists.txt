# cmake最低版本
cmake_minimum_required(VERSION 3.14)
# 本项目名
project(local-ipc-demo-pro)

# 设置C++17标准，需要满足标准，不用C++拓展语法
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (OS_WIN)
    set(Qt5_DIR "D:\\Qt\\5.15.2\\msvc2019_64\\lib\\cmake\\Qt5")
else()
    set(QT_PATH /opt/Qt/5.15.2/gcc_64)
    set(CMAKE_PREFIX_PATH ${QT_PATH})
    message("-- The qt path:${CMAKE_PREFIX_PATH}")

    set(Qt5_DIR "${CMAKE_PREFIX_PATH}/lib/cmake/Qt5")
    message("The qt5 dir:${Qt5_DIR}")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

set(CMAKE_BUILD_TYPE "Release")


# 进入rs_logger文件夹下面的CMakeLists.txt
add_subdirectory(logger)

# 进入ripcqccomponent文件夹下面的CMakeLists.txt
add_subdirectory(ripcqccomponent)

# 进入rpc_client_demo文件夹下面的CMakeLists.txt
add_subdirectory(rpc_client_demo)

if (OS_WIN)
	# 进入rpc_demo_srv文件夹下面的CMakeLists.txt
	add_subdirectory(rpc_srv_demo)
else()
endif()
