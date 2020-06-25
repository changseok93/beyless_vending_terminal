cmake_minimum_required(VERSION 3.13)
project(daemon_process)

set(CMAKE_CXX_STANDARD 17)
set(PAHO_CPP_LIB paho-mqttpp3)
set(PAHO_C_LIB paho-mqtt3a)

find_package( OpenCV REQUIRED )
add_executable(daemon_process main.cpp doorLock.cpp doorLock.h camera.cpp camera.h terminal.cpp terminal.h logger.cpp logger.h)
target_link_libraries( daemon_process ${OpenCV_LIBS} ${PAHO_C_LIB} ${PAHO_CPP_LIB} pthread curl mysqlclient)

add_definitions(-DOPENSSL)

