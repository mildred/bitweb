#ifndef BITWEB_APPLICATION_SERVER_H
#define BITWEB_APPLICATION_SERVER_H

#include <QObject>
#include "signal_dispatcher.h"
#include "torrent_server.h"
#include "server.h"

namespace bitweb {

class application_server : public QObject
{
    Q_OBJECT
public:
    explicit application_server(int port, QObject *parent = 0);
    void setDebug(bool d);

    void onSIGUSR1();

signals:

public slots:

private:
    signal_dispatcher<SIGTERM> sigusr1;

    torrent_server torrent_srv;
    server         http_srv;

};

} // namespace bitweb

#endif // BITWEB_APPLICATION_SERVER_H
