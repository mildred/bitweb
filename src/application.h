#ifndef BITWEB_APPLICATION_H
#define BITWEB_APPLICATION_H

#include <QCoreApplication>
#include "signal_dispatcher.h"
#include "server.h"
#include "torrent_server.h"

namespace bitweb {

class application_server;
class application_show;
class application_update;

class application : public QCoreApplication
{
    Q_OBJECT

public:
    explicit application(int &argc, char **argv);
    virtual ~application();

    bool parseArguments();

    int exec();

signals:

public slots:

private:
    void generateKeyPair(QString filename);

    bool run;

    signal_dispatcher<SIGINT>  sigint;
    signal_dispatcher<SIGTERM> sigterm;

    application_server *server;
    application_show   *show;
    application_update *update;
};

} // namespace bitweb

#endif // BITWEB_APPLICATION_H
