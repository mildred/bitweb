#ifndef BITWEB_APPLICATION_TOOL_H
#define BITWEB_APPLICATION_TOOL_H

#include <QObject>
#include <QFile>

#include <cryptopp/rsa.h>

namespace bitweb {

class application_show : public QObject
{
    Q_OBJECT
public:
    explicit application_show(QString torrent, QObject *parent = 0);

    int exec();

private:

    QFile _torrent;
    CryptoPP::RSASSA_PKCS1v15_SHA_Verifier _verifier;
};

} // namespace bitweb

#endif // BITWEB_APPLICATION_TOOL_H
