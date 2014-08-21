#include <functional>

#include <QDebug>
#include <QUrl>
#include <QUrlQuery>
#include <QTcpSocket>
#include <Tufao/HttpServerRequest>
#include <Tufao/Headers>
#include <Tufao/HttpServerResponse>
#include "server.h"

#define DOMAIN_NAME_EXTENSION     ".bitweb"
#define DOMAIN_NAME_EXTENSION_LEN 7

namespace bitweb {

using namespace std::placeholders;

server::server(torrent_server *torrent, QObject *parent) :
    Tufao::HttpServer(parent),
    _socks(std::bind(&server::acceptSOCKSRequest, this, _1, _2, _3)),
    _torrent(torrent)
{

    connect(this,    SIGNAL(requestReady(Tufao::HttpServerRequest&,Tufao::HttpServerResponse&)),
            this,    SLOT(handleHTTPRequst(Tufao::HttpServerRequest&,Tufao::HttpServerResponse&)));
    connect(&_socks, SIGNAL(connectionRequested(QTcpSocket*,QByteArray,qint16)),
            this,    SLOT(handleSOCKSRequest(QTcpSocket*,QByteArray)));
}

void server::listen(quint16 port)
{
    _socks.listen(port);
}

void server::handleSOCKSRequest(QTcpSocket *sock, const QByteArray &domainName)
{
    QByteArray infoHash = domainName;
    infoHash.resize(domainName.size() - DOMAIN_NAME_EXTENSION_LEN);
    infoHash = QByteArray::fromHex(infoHash);
    _torrent->prepare(infoHash);
    sock->setProperty("bitweb.infohash", infoHash);
    this->handleConnection(sock);
}

void server::handleHTTPRequst(Tufao::HttpServerRequest &req, Tufao::HttpServerResponse &res)
{
    QByteArray infoHash = req.socket().property("bitweb.infohash").value<QByteArray>();

    auto f = _torrent->get_file(infoHash, req.url().path());
    if(!f->exists) {
        res.writeHead(404, "Not Found");
        res.end();
        return;
    }

    res.writeHead(200, "OK");

    QUrlQuery q(req.url());
    if(q.hasQueryItem("content-type")){
        res.headers().replace("Content-Type", q.queryItemValue("content-type").toUtf8());
    }

    _torrent->stream_file(f, [&res](QByteArray data){
        qDebug() << "stream" << data.size() << "bytes";
        res.write(data);
    });
    res.end();
}

int server::acceptSOCKSRequest(QByteArray domainName, qint16 portNumber, QByteArray userid)
{
    return domainName.endsWith(DOMAIN_NAME_EXTENSION) && domainName.size() == 40 + DOMAIN_NAME_EXTENSION_LEN;
}

} // namespace bitweb
