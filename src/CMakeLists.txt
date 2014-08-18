#find_package(Boost REQUIRED COMPONENTS program_options)
#include_directories(${Boost_INCLUDE_DIRS})

#find_package(Qt5 COMPONENTS Widgets)

#find_package(Tufao1 1.0 REQUIRED)
#include_directories("${TUFAO_INCLUDE_DIR}")

#pkg_check_modules(BITTORRENT REQUIRED libtorrent-rasterbar)
#link_directories(${BITTORRENT_LIBRARY_DIRS})
#include_directories(${BITTORRENT_INCLUDE_DIRS})
#add_definitions(${BITTORRENT_CFLAGS})

#add_executable(bitweb
#  bitweb.cpp
#)

#target_link_libraries(bitweb ${BITTORRENT_LIBRARIES} ${TUFAO_LIBRARIES} Qt5::Widgets)
#target_link_libraries(bitweb ${Boost_LIBRARIES} socks4a)
