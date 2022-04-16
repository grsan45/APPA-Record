cmake_minimum_required(VERSION 3.10.2   )
project(APPA_record)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig)
pkg_check_modules(PC_ZeroMQ QUIET zmq)

find_path(ZeroMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )

find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(OpenCV REQUIRED)
include_directories(${OPENCV_INCLUDE_DIRS} ${ZeroMQ_INCLUDE_DIRS})

add_executable(APPA_record main.cpp CaptureDevice.cpp CaptureDevice.h utils.h)
target_link_libraries(APPA_record ${OpenCV_LIBS} ${ZeroMQ_LIBRARY} Threads::Threads stdc++fs)
target_compile_definitions(APPA_record PUBLIC ZMQ_ENABLED)

add_executable(APPA_record_standalone main.cpp CaptureDevice.cpp CaptureDevice.h utils.h)
target_link_libraries(APPA_record_standalone ${OpenCV_LIBS} ${ZeroMQ_LIBRARY} Threads::Threads stdc++fs)