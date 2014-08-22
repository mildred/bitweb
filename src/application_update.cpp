#include <QCryptographicHash>
#include <QSslKey>
#include <QDebug>

#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/file.hpp>
#include <libtorrent/file_pool.hpp>
#include <libtorrent/storage.hpp>
#include <libtorrent/hasher.hpp>
#include <libtorrent/create_torrent.hpp>
#include <libtorrent/ed25519.hpp>

#include "application_update.h"
#include "application.h"
#include "qstdstream.h"

namespace {

bool operator == (QStringList a, QStringList b) {
    int len = a.length();
    if(len != b.length()) return false;
    for(int i = 0; i < len; ++i) {
        if(a[i] != b[i]) return false;
    }
    return true;
}

}


namespace bitweb {

constexpr const char *creator_str = "bitweb";

application_update::application_update(QString torrent, bool truncate, application *parent) :
    QObject(parent),
    _app(parent),
    _torrentFile(torrent),
    _truncate(truncate),
    _piece_size(16 * 1024)
{
}

void application_update::setCreationDirectory(QString dir)
{
    _creation_dir = dir;
}

bool application_update::setKeyFile(QString filename)
{
    _app->readKeyFile(filename, &_pubKey);
}

bool application_update::open()
{
    if(!_truncate) {
        _readTorrent();
    } else if(!_creation_dir.isEmpty()) {
        QByteArray parent_path = QFileInfo(_creation_dir).path().toUtf8();
        using namespace libtorrent;

        libtorrent::file_storage fs;
        fs.set_piece_length(_piece_size);

        _addDir(fs, _creation_dir);

        if(fs.num_files() == 0) {
            file_entry e;
            e.path = "bitwebsite/.bitweb";
            e.size = 0;
            e.pad_file = true;
            fs.add_file(e);
        }

        QByteArray fileName = QFileInfo(_creation_dir).fileName().toUtf8();
        fs.set_name(fileName.constData());

        libtorrent::create_torrent tc(fs, _piece_size, -1, 0);
        libtorrent::set_piece_hashes(tc, parent_path.constData());
        tc.set_creator(creator_str);

        // create the torrent and print it to out
        //std::cout << t.generate() << std::endl;
        _torrent = tc.generate();
    } else {
        QFile err;
        err.open(stderr, QIODevice::WriteOnly);
        err.write("Cannot create torrent from non existing directory");
        return false;
    }

    return true;
}

void application_update::_readTorrent()
{
    _torrentFile.open(QIODevice::ReadOnly);
    QByteArray torrent_bytes = _torrentFile.readAll();
    _torrent = libtorrent::bdecode(torrent_bytes.begin(), torrent_bytes.end());
    _torrentFile.close();

    _piece_size = -1;
}

void application_update::_addDir(libtorrent::file_storage &fs, QDir d, QStringList prefix)
{
    for(QFileInfo f : d.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
        prefix.append(f.fileName());
        if(f.isDir()) {
            _addDir(fs, f.absoluteFilePath(), prefix);
        } else {
            _addFile(fs, f, prefix);
        }
        prefix.removeLast();
    }
}

void application_update::_addFile(libtorrent::file_storage &fs, QFileInfo f, QStringList path)
{
    Q_ASSERT(_piece_size > 0);
    using namespace libtorrent;

    file_entry e;
    QByteArray entry_path = "bitwebsite/" + path.join("/").toUtf8();
    e.path = entry_path.constData();
    e.size = f.size();
    fs.add_file(e);

    unsigned pad_size = _piece_size - (e.size % _piece_size);
    if(pad_size == _piece_size || pad_size == 0) return;

    QByteArray entry_pad_path = "bitwebsite/.padding/" + path.join("/").toUtf8();
    e.path = entry_pad_path.constData();
    e.size = pad_size;
    e.pad_file = true;
    fs.add_file(e);
}

bool application_update::setHeader(QString path, QString header, QString value)
{
    QStringList pathComp = path.split("/", QString::SkipEmptyParts);
    libtorrent::entry &info = _torrent["info"];
    libtorrent::entry *file_entry_ptr = nullptr;
    if(path == "/" && info.find_key("length")) {
        // Single file torrent
        file_entry_ptr = &info;
    } else if(info.find_key("files")) {
        libtorrent::entry &files = info["files"];
        for(libtorrent::entry &entry : files.list()) {
            QStringList curPathComp;
            for(libtorrent::entry &comp : entry["path"].list()) {
                curPathComp << QString::fromStdString(comp.string());
            }
            if(curPathComp == pathComp) {
                file_entry_ptr = &entry;
                break;
            }
        }
    }
    if(file_entry_ptr) {
        libtorrent::entry &file_entry = *file_entry_ptr;
        libtorrent::entry *headers_ptr = file_entry.find_key("headers");
        if(!headers_ptr){
            file_entry["headers"] = libtorrent::entry(libtorrent::entry::dictionary_t);
            headers_ptr = &file_entry["headers"];
        }
        Q_ASSERT(headers_ptr);
        libtorrent::entry &headers = *headers_ptr;
        headers[header.toStdString()] = value.toStdString();
    }
}

int application_update::exec()
{
    _writeTorrent();
    return 0;
}

void application_update::_writeVersion()
{
    typedef libtorrent::entry entry;

    entry &info = _torrent["info"];
    if(!info.find_key("version")) info["version"] = entry(entry::dictionary_t);
    entry &version = info["version"];

    if(!_pubKey.isEmpty()) {
        version["public key"] = entry(str(_pubKey));
    } else if(version.find_key("public key")) {
        _pubKey = qt(version["public key"].string());
    }
    if(!version.find_key("id")){
        unsigned char seed[ed25519_seed_size];
        ed25519_create_seed(seed);
        version["id"] = entry(std::string((char*)seed, ed25519_seed_size));
    }
    if(!version.find_key("rev")){
        version["rev"] = entry(1);
    }
    if(!version.find_key("parents")){
        version["parents"] = entry(entry::list_t);
    }
}

void application_update::_writeTorrent()
{
    _writeVersion();

    // Sign torrent

    if(!_pubKey.isEmpty()) {
        unsigned char sign[ed25519_signature_size];
        QByteArray signatureBuffer;
        QByteArray priv = _app->findPrivateKey(_pubKey);
        libtorrent::bencode(std::back_inserter(signatureBuffer), _torrent["info"]);
        ed25519_sign(sign, (const unsigned char*) signatureBuffer.constData(), signatureBuffer.size(),
                     (const unsigned char*) _pubKey.constData(),
                     (const unsigned char*) priv.constData());
        _torrent["version signature"] = libtorrent::entry(std::string((char*)sign, ed25519_signature_size));
    }

    #if 0
    std::cout << te << std::endl;
    te.print(std::cout);
    #endif

    // Write to file

    QByteArray resBuffer;
    libtorrent::bencode(std::back_inserter(resBuffer), _torrent);
    _torrentFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    _torrentFile.write(resBuffer);
    _torrentFile.close();
}

} // namespace bitweb
