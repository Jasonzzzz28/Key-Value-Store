cmake_minimum_required(VERSION 3.14)
project(kv_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(kv_server
  src/main.cpp
  src/net/tcp_server.cpp
  src/net/connection.cpp
  src/protocol/parser.cpp
  src/protocol/response.cpp
  src/core/dispatcher.cpp
  src/core/commands.cpp
  src/storage/kv_store.cpp
)

target_include_directories(kv_server PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(UNIX AND NOT APPLE)
  target_link_libraries(kv_server PRIVATE pthread)
endif()
