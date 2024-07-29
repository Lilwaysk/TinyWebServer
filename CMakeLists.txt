cmake_minimum_required(VERSION 3.22)

project(TinyWebServer)

include_directories(/usr/include/mysql)
include_directories(/usr/local/include/mysql++)
include_directories(/usr/lib/x86_64-linux-gnu)

add_executable(test test.cpp buffer.cpp log.cpp sqlconnpool.cpp)

target_link_libraries(test mysqlclient)