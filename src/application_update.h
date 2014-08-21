#ifndef BITWEB_APPLICATION_UPDATE_H
#define BITWEB_APPLICATION_UPDATE_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDir>
#include <QList>
#include <QIODevice>

#include <libtorrent/hasher.hpp>

#include <cryptopp/rsa.h>

namespace libtorrent { class file_storage; }

namespace bitweb {

class application_update : public QObject
{
    Q_OBJECT
public:
    application_update(QString torrent, bool truncate, QObject *parent = 0);

    void setCreationDirectory(QString dir);
    bool setKeyFile(QString filename);
    int exec();

private:
    void _readTorrent();
    void _addDir(libtorrent::file_storage &fs, QDir d, QStringList prefix = QStringList());
    void _addFile(libtorrent::file_storage &fs, QFileInfo f, QStringList path);
    void _pad(libtorrent::file_storage &fs, unsigned piece_size);
    void _hash(const QByteArray &data);
    void _hash(QIODevice *f);

    QString  _creation_dir;
    QFile    _torrent;
    bool     _truncate;
    unsigned _piece_size;
    unsigned _hasher_size;
    libtorrent::hasher _hasher;
    QList<libtorrent::sha1_hash> _hashs;
    CryptoPP::RSA::PrivateKey _privateKey;
    CryptoPP::RSASSA_PKCS1v15_SHA_Signer _signer;
};

} // namespace bitweb

#endif // BITWEB_APPLICATION_UPDATE_H
