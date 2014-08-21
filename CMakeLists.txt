cmake_minimum_required(VERSION 3.0)
project(bitweb)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_AUTOMOC   TRUE)

include(FindPkgConfig)

find_package(OpenSSL REQUIRED)
find_package(Qt5 COMPONENTS Core Network)
find_package(Tufao1 1.0 REQUIRED)
pkg_check_modules(BITTORRENT REQUIRED libtorrent-rasterbar)
pkg_check_modules(CRYPTOPP REQUIRED libcrypto++)

add_subdirectory(src)

include_directories("${TUFAO_INCLUDE_DIR}")
link_directories(${BITTORRENT_LIBRARY_DIRS})
include_directories(${BITTORRENT_INCLUDE_DIRS})
add_definitions(${BITTORRENT_CFLAGS})

add_executable(bitweb
  src/bitweb.cpp
)

target_link_libraries(bitweb ${BITTORRENT_LIBRARIES} ${TUFAO_LIBRARIES} ${CRYPTOPP_LIBRARIES} crypto Qt5::Core dl bitweb_server)
