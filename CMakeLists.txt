cmake_minimum_required(VERSION 3.0)
project(bitweb)
include(FindPkgConfig)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(src)



find_package(Qt5 COMPONENTS Core)

find_package(Tufao1 1.0 REQUIRED)
include_directories("${TUFAO_INCLUDE_DIR}")

pkg_check_modules(BITTORRENT REQUIRED libtorrent-rasterbar)
link_directories(${BITTORRENT_LIBRARY_DIRS})
include_directories(${BITTORRENT_INCLUDE_DIRS})
add_definitions(${BITTORRENT_CFLAGS})

add_executable(bitweb
  src/bitweb.cpp
)

target_link_libraries(bitweb ${BITTORRENT_LIBRARIES} ${TUFAO_LIBRARIES} Qt5::Core)
#target_link_libraries(bitweb ${Boost_LIBRARIES} socks4a)

