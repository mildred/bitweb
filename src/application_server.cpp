#include <functional>
#include "application_server.h"

namespace bitweb {

application_server::application_server(int port, QObject *parent) :
    QObject(parent),
    sigusr1(std::bind(&application_server::onSIGUSR1, this)),
    torrent_srv(),
    http_srv(&torrent_srv)
{
    http_srv.listen(port);
}

void application_server::setDebug(bool d)
{
    torrent_srv.setDebug(d);
}

void application_server::onSIGUSR1()
{
    torrent_srv.print_status();
}

} // namespace bitweb
