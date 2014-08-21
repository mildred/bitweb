#ifndef BITWEB_CRYPTO_RSA_H
#define BITWEB_CRYPTO_RSA_H

#include <openssl/rsa.h>
#include <QObject>

// Thank you Retorut
// https://www.opensc.ws/tutorials-and-articles/12761-rsa-encryption-using-openssl-c-c.html
// Thank you blanclux
// http://sehermitage.web.fc2.com/program/src/rsacrypt.c

namespace bitweb {
namespace crypto {

RSA *readPublicRSAKey(QByteArray key);
RSA *readPrivateRSAKey(QByteArray key);

QByteArray sslSign  (RSA *rsa, QByteArray hash_sha256);
bool       sslVerify(RSA *rsa, QByteArray hash_sha256, QByteArray signature);

#if 0

class RSA : public QObject
{
    Q_OBJECT
public:
    explicit RSA(QObject *parent = 0);
    virtual ~RSA();

    void readPublicKey(QByteArray key);
    void readPrivateKey(QByteArray key);
    QByteArray encrypt(QByteArray message);
    QByteArray decrypt(QByteArray message);

private
    RSA *_rsa;
};

#endif

} // namespace crypto
} // namespace bitweb

#endif // BITWEB_CRYPTO_RSA_H
