#ifndef BITWEB_APPLICATION_UPDATE_H
#define BITWEB_APPLICATION_UPDATE_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDir>
#include <QList>
#include <QIODevice>

#include <libtorrent/hasher.hpp>

namespace libtorrent { class file_storage; }

namespace bitweb {

class application;

class application_update : public QObject
{
    Q_OBJECT
public:
    application_update(QString torrent, bool truncate, application *parent = 0);

    void setCreationDirectory(QString dir);
    bool setKeyFile(QString filename);
    bool setHeader(QString path, QString header, QString value);
    bool open();
    int exec();

private:
    void _readTorrent();
    void _addDir(libtorrent::file_storage &fs, QDir d, QStringList prefix = QStringList());
    void _addFile(libtorrent::file_storage &fs, QFileInfo f, QStringList path);
    void _pad(libtorrent::file_storage &fs, unsigned piece_size);
    void _writeVersion();
    void _writeTorrent();

    application *_app;
    QString  _creation_dir;
    QFile    _torrentFile;
    bool     _truncate;
    unsigned _piece_size;
    QByteArray _pubKey;
    libtorrent::entry _torrent;
};

} // namespace bitweb

#endif // BITWEB_APPLICATION_UPDATE_H
