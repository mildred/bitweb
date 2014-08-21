#ifndef __BITWEB_SOCKS4A_SOCKS4A_H__
#define __BITWEB_SOCKS4A_SOCKS4A_H__

#include <functional>
#include <QObject>
#include <QTcpServer>

// FIXME: make it generic for socks4a and socks5

namespace bitweb {
namespace socks4a {

class session;

class server : public QObject {
    Q_OBJECT
public:

    server(std::function<int(QByteArray domainName, qint16 portNumber, QByteArray userid)> accept);
    virtual ~server();

    void listen(int port);

signals:

    void connectionRequested(QTcpSocket *sock, QByteArray domainName, qint16 portNumber);

private slots:

    void onConnection();
    void onConnectionRequested(session *session, QByteArray domainName, qint16 portNumber, QByteArray userid);

private:

    std::function<int(QByteArray domainName, qint16 portNumber, QByteArray userid)> _accept;
    QTcpServer _tcp;
};

class session : public QObject {
    Q_OBJECT

public:

    session(QTcpSocket *socket);
    virtual ~session();

    void handleConnection();

    QTcpSocket *grantRequest();
    void        rejectRequest(qint8 cd = 91);

signals:

    void connectionRequested(session *session, QByteArray domainName, qint16 portNumber, QByteArray userid);

private:

    void handleError(qint8 cd = 91);

    qint16 _dstport;
    qint32 _dstip;
    QTcpSocket *_sock;
};

}
}

#endif
