add_subdirectory(socks4a)

include_directories(${Qt5Network_INCLUDE_DIRS}) # QtCreator
include_directories("${TUFAO_INCLUDE_DIR}" ${BITTORRENT_INCLUDE_DIRS})
add_definitions(${BITTORRENT_CFLAGS})
link_directories(${BITTORRENT_LIBRARY_DIRS})

add_library(bitweb_server
    deathhandler/death_handler.cc
    server.cpp
    torrent_server.cpp
    application.cpp
    application_server.cpp
    application_show.cpp
    application_update.cpp
    socks4a/socks4a.cpp
)
target_link_libraries(bitweb_server Qt5::Core Qt5::Network ${BITTORRENT_LIBRARIES})

