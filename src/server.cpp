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

    Tufao::HttpResponseStatus status = (Tufao::HttpResponseStatus) f->http_status;
    res.writeHead(status, reasonPhrase(status, f->http_status_message));

    if(f->headers) {
        int num_headers = f->headers->dict_size();
        for(int i = 0; i < num_headers; ++i) {
            std::pair<std::string, libtorrent::lazy_entry const*> keyval = f->headers->dict_at(i);
            QByteArray key(keyval.first.c_str(), keyval.first.size());
            QByteArray val(keyval.second->string_value().c_str(), keyval.second->string_length());
            res.headers().replace(key, val);
        }
    }

    QUrlQuery q(req.url());
    if(q.hasQueryItem("content-type")){
        res.headers().replace("Content-Type", q.queryItemValue("content-type").toUtf8());
    }

    if(!f->exists) {
        res.end();
        return;
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

QByteArray server::reasonPhrase(Tufao::HttpResponseStatus status, std::string defaultReason)
{
    QByteArray message(defaultReason.c_str(), defaultReason.length());
    if(!message.isEmpty()) return message;

#define case(x) case Tufao::HttpResponseStatus::x: message = #x; break;
    switch(status){
    // 1xx Informational
    case(CONTINUE)
    case(SWITCHING_PROTOCOLS)
    case(PROCESSING)
    case(CHECKPOINT)
    // 2xx Successful
    case(OK)
    case(CREATED)
    case(ACCEPTED)
    case(NON_AUTHORITATIVE_INFORMATION)
    case(NO_CONTENT)
    case(RESET_CONTENT)
    case(PARTIAL_CONTENT)
    case(MULTI_STATUS)
    case(ALREADY_REPORTED)
    case(IM_USED)
    // 3xx Redirection
    case(MULTIPLE_CHOICES)
    case(MOVED_PERMANENTLY)
    case(FOUND)
    case(SEE_OTHER)
    case(NOT_MODIFIED)
    case(USE_PROXY)
    case(SWITCH_PROXY)
    case(TEMPORARY_REDIRECT)
    case(RESUME_INCOMPLETE)
    // 4xx Client Error
    case(BAD_REQUEST)
    case(UNAUTHORIZED)
    case(PAYMENT_REQUIRED)
    case(FORBIDDEN)
    case(NOT_FOUND)
    case(METHOD_NOT_ALLOWED)
    case(NOT_ACCEPTABLE)
    case(PROXY_AUTHENTICATION_REQUIRED)
    case(REQUEST_TIMEOUT)
    case(CONFLICT)
    case(GONE)
    case(LENGTH_REQUIRED)
    case(PRECONDITION_FAILED)
    case(REQUEST_ENTITY_TOO_LARGE)
    case(REQUEST_URI_TOO_LONG)
    case(UNSUPPORTED_MEDIA_TYPE)
    case(REQUESTED_RANGE_NOT_SATISFIABLE)
    case(EXPECTATION_FAILED)
    case(I_AM_A_TEAPOT)
    case(UNPROCESSABLE_ENTITY)
    case(LOCKED)
    case(FAILED_DEPENDENCY)
    case(UNORDERED_COLLECTION)
    case(UPGRADE_REQUIRED)
    case(PRECONDITION_REQUIRED)
    case(TOO_MANY_REQUESTS)
    case(REQUEST_HEADER_FIELDS_TOO_LARGE)
    case(NO_RESPONSE)
    case(RETRY_WITH)
    case(CLIENT_CLOSED_REQUEST)
    // 5xx Internal Server Error
    case(INTERNAL_SERVER_ERROR)
    case(NOT_IMPLEMENTED)
    case(BAD_GATEWAY)
    case(SERVICE_UNAVAILABLE)
    case(GATEWAY_TIMEOUT)
    case(HTTP_VERSION_NOT_SUPPORTED)
    case(VARIANT_ALSO_NEGOTIATES)
    case(INSUFFICIENT_STORAGE)
    case(LOOP_DETECTED)
    case(BANDWIDTH_LIMIT_EXCEEDED)
    case(NOT_EXTENDED)
    }
#undef case

    for(int i = 0; i < message.size(); ++i) {
        if(message[i] == '_') message[i] == ' ';
        if(i == 0 || message[i-1] == ' ') {
            if(message[i] >= 'a' && message[i] <= 'z') message[i] = message[i] + 'A' - 'a';
        } else if(message[i] >= 'A' && message[i] <= 'Z') message[i] = message[i] + 'a' - 'A';
    }

    return message;
}

} // namespace bitweb
