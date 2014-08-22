add_subdirectory(socks4a)

include_directories(${Qt5Network_INCLUDE_DIRS} ed25519/src) # QtCreator
include_directories("${TUFAO_INCLUDE_DIR}" ${BITTORRENT_INCLUDE_DIRS} ${CRYPTOPP_INCLUDE_DIRS})
add_definitions(${BITTORRENT_CFLAGS} ${CRYPTOPP_CFLAGS})
link_directories(${BITTORRENT_LIBRARY_DIRS} ${CRYPTOPP_LIBRARY_DIRS})

add_library(bitweb_server
    deathhandler/death_handler.cc
    server.cpp
    torrent_server.cpp
    application.cpp
    application_server.cpp
    application_show.cpp
    application_update.cpp
    socks4a/socks4a.cpp
    ed25519/src/add_scalar.c
    ed25519/src/fe.c
    ed25519/src/ge.c
    ed25519/src/key_exchange.c
    ed25519/src/keypair.c
    ed25519/src/sc.c
    ed25519/src/seed.c
    ed25519/src/sha512.c
    ed25519/src/sign.c
    ed25519/src/verify.c
)
target_link_libraries(bitweb_server Qt5::Core Qt5::Network ${BITTORRENT_LIBRARIES} ${CRYPTOPP_LIBRARIES})

