# QtCreator
include_directories(${Qt5Network_INCLUDE_DIRS})

add_library(socks4a socks4a.cpp)
target_link_libraries(socks4a Qt5::Core Qt5::Network)

