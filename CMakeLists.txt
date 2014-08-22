cmake_minimum_required(VERSION 3.0)
project(bitweb)

set(CMAKE_CXX_FLAGS      "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated-declarations")
set(CMAKE_AUTOMOC        TRUE)
set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/install/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

include(FindPkgConfig)

find_package(OpenSSL REQUIRED)
find_package(Qt5 COMPONENTS Core Network)
find_package(Tufao1 1.0 REQUIRED)
pkg_check_modules(BITTORRENT REQUIRED libtorrent-rasterbar)

add_subdirectory(src)

include_directories("${TUFAO_INCLUDE_DIR}")
link_directories(${BITTORRENT_LIBRARY_DIRS})
include_directories(${BITTORRENT_INCLUDE_DIRS})
add_definitions(${BITTORRENT_CFLAGS})

add_executable(bitweb
  src/bitweb.cpp
)

target_link_libraries(bitweb ${BITTORRENT_LIBRARIES} ${TUFAO_LIBRARIES} Qt5::Core dl bitweb_server)
