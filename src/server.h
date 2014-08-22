#ifndef BITWEB_SERVER_H
#define BITWEB_SERVER_H

#include <QObject>
#include <Tufao/HttpServer>
#include <Tufao/HttpServerResponse>
#include "socks4a/socks4a.h"
#include "torrent_server.h"

namespace bitweb {

// FIXME: rename to sockshttp_server
class server : public Tufao::HttpServer
{
    Q_OBJECT
public:
    explicit server(torrent_server *torrent, QObject *parent = 0);

    void listen(quint16 port);

signals:

public slots:

private slots:

    void handleSOCKSRequest(QTcpSocket *session, const QByteArray &domainName);
    void handleHTTPRequst(Tufao::HttpServerRequest &req, Tufao::HttpServerResponse &res);

private:
    int acceptSOCKSRequest(QByteArray domainName, qint16 portNumber, QByteArray userid);

    QByteArray reasonPhrase(Tufao::HttpResponseStatus status, std::string defaultReason);

    socks4a::server _socks;
    torrent_server *_torrent;
};

} // namespace bitweb

#endif // BITWEB_SERVER_H
