#include <QDebug>
#include <QTcpSocket>
#include <QtEndian>
#include "socks4a.h"

namespace bitweb {
namespace socks4a {

server::server(std::function<int(QByteArray domainName, qint16 portNumber, QByteArray userid)> accept) :
    _accept(accept), _tcp()
{
    connect(&_tcp, SIGNAL(newConnection()), this, SLOT(onConnection()));
}

server::~server()
{
}

void server::listen(int port)
{
    if(!_tcp.listen(QHostAddress::LocalHost, port)) {
        qFatal("Could not listen to  %d (SOCKS proxy port)", port);
    }
}

void server::onConnection()
{
    QTcpSocket *clientConnection = _tcp.nextPendingConnection();
    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater()));
    session *sess = new session(clientConnection);
    connect(sess, SIGNAL(connectionRequested(session*,QByteArray,qint16,QByteArray)),
            this, SLOT(onConnectionRequested(session*,QByteArray,qint16,QByteArray)));
    sess->handleConnection();
}

void server::onConnectionRequested(session *session, QByteArray domainName, qint16 portNumber, QByteArray userid)
{
    int res = _accept(domainName, portNumber, userid);
    if(res == 90 || (res && res < 90) || (res && res > 93)) {
        QTcpSocket *sock = session->grantRequest();
        emit connectionRequested(sock, domainName, portNumber);
    } else {
        if(res >= 90 && res <= 93) session->rejectRequest(res);
        else session->rejectRequest();
    }
}

session::session(QTcpSocket *socket) : QObject(socket), _sock(socket)
{
}

session::~session()
{
}

QTcpSocket *session::grantRequest()
{
    handleError(90);
    return _sock;
}

void session::rejectRequest(qint8 cd)
{
    return handleError(cd);
}

void session::handleConnection()
{
    quint8 vn, cd;
    qint16 dstport;
    qint32 dstip;
    QByteArray userid, domain;
    char c;

    _sock->read((char*) &vn, 1);
    if(vn != 4) return handleError();

    _sock->read((char*) &cd, 1);
    if(cd != 1) return handleError();


    _sock->read((char*) &dstport, 2); _dstport = qFromBigEndian(dstport);
    _sock->read((char*) &dstip, 4);   _dstip   = qFromBigEndian(dstip);
    if((_dstip & 0xFFFFFF00) != 0 || _dstip == 0) return handleError();

    do {
        _sock->read(&c, 1);
        if(c) userid.append(c);
    } while(c);

    do {
        _sock->read(&c, 1);
        if(c) domain.append(c);
    } while(c);

    //qDebug() << "SOCKS" << vn << cd << dstport << dstip << userid << domain;

    emit connectionRequested(this, domain, dstport, userid);
}

void session::handleError(qint8 cd)
{
    quint8 vn = 0;
    qint16 dstport = qToBigEndian(_dstport);
    qint32 dstip   = qToBigEndian(_dstip);
    _sock->write((char*) &vn, 1);
    _sock->write((char*) &cd, 1);
    _sock->write((char*) &dstport, 2);
    _sock->write((char*) &dstip, 4);
    if(cd != 90) _sock->close();
}


}
}
