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

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <cryptopp/files.h>
#include <cryptopp/base64.h>

#include "application_update.h"


namespace bitweb {

namespace {

void putData(CryptoPP::BufferedTransformation &bt, QByteArray data) {
    bt.Put((const unsigned char*) data.constData(), data.size());
    bt.MessageEnd();
}

void putData(CryptoPP::BufferedTransformation &bt, QIODevice *io) {
    while(!io->atEnd()) {
        QByteArray data = io->read(1024 * 16);
        bt.Put((const unsigned char*) data.constData(), data.size());
    }
    bt.MessageEnd();
}

QByteArray getData(CryptoPP::BufferedTransformation &bt) {
    CryptoPP::lword len = bt.MaxRetrievable();
    QByteArray res(len, 0);
    bt.Get((unsigned char*) res.data(), len);
    return res;
}

}

constexpr const char *creator_str = "bitweb";

application_update::application_update(QString torrent, bool truncate, QObject *parent) :
    QObject(parent),
    _torrent(torrent),
    _truncate(truncate),
    _piece_size(16 * 1024),
    _hasher_size(0),
    _hasher(),
    _hashs()
{
}

void application_update::setCreationDirectory(QString dir)
{
    _creation_dir = dir;
}

bool application_update::setKeyFile(QString filename)
{
    try {
        CryptoPP::ByteQueue bytes;
        QFile f(filename);
        f.open(QIODevice::ReadOnly);
        //QSslKey k(&f, QSsl::Rsa);
        putData(bytes, &f);
        f.close();
        //putData(bytes, k.toDer());
        //putData(bytes, QByteArray::fromBase64(""));
        //_privateKey.Load(bytes);
        bytes.MessageEnd();
        _signer.AccessKey().Load(bytes);
        return true;
    } catch(...) {
        QFileInfo f(filename);
        QByteArray openssl = QString("\topenssl pkcs8 -in %1 -out %2.p8.der -topk8 -nocrypt -outform der")
                .arg(filename)
                .arg(f.dir().filePath(f.baseName()))
                .toLocal8Bit();
        qCritical() << "Could not load private key, check that it is in PKCS#8 DER format.";
        qCritical() << "To convert PKCS#1 to PKCS#8, use:";
        qCritical() << openssl.constData();
        return false;
    }

    /*
    QByteArray encodedFileName = QFile::encodeName(filename);
    CryptoPP::ByteQueue bytes;
    //CryptoPP::Base64Decoder decoder;
    CryptoPP::FileSource file(encodedFileName.constData(), true);//, &decoder);
    file.TransferTo(bytes);
    bytes.MessageEnd();
    _privateKey.Load(bytes);
    */
}

int application_update::exec()
{
    if(!_truncate) {
        _readTorrent();
        _piece_size = -1;
    }
    if(!_creation_dir.isEmpty()) {
        QByteArray full_path   = _creation_dir.toUtf8();
        QByteArray parent_path = QFileInfo(_creation_dir).path().toUtf8();
        using namespace libtorrent;
        using namespace boost::filesystem;

        libtorrent::file_storage fs;
        fs.set_piece_length(_piece_size);
        //libtorrent::add_files(fs, full_path.constData());
        //_pad(fs, _piece_size);


        /*QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData("", 0);
        QByteArray sha1 = hash.result();*/

        _addDir(fs, _creation_dir);

        if(fs.num_files() == 0) {
            file_entry e;
            e.path = "bitwebsite/.bitweb";
            e.size = 0;
            e.pad_file = true;
            fs.add_file(e); //, sha1.constData());
        }

        Q_ASSERT(_hasher_size == 0);

        QByteArray fileName = QFileInfo(_creation_dir).fileName().toUtf8();
        fs.set_name(fileName.constData());
        fs.set_num_pieces(_hashs.size());

        libtorrent::create_torrent t(fs, _piece_size, -1, 0);
        //for(int i = 0; i < _hashs.size(); ++i) {
        //    t.set_hash(i, _hashs[i]);
        //}
        libtorrent::set_piece_hashes(t, parent_path.constData());
        t.set_creator(creator_str);

        // create the torrent and print it to out
        //std::cout << t.generate() << std::endl;
        QByteArray resBuffer;
        libtorrent::entry te = t.generate();
        CryptoPP::ByteQueue bytes;
        //_privateKey.DEREncodePublicKey(bytes);
        _signer.AccessKey().DEREncodePublicKey(bytes);
        QByteArray der = getData(bytes);
        te["info"]["public key"] = std::string(der.constData(), der.size());
        te["info"].dict().erase("signature");
        libtorrent::bencode(std::back_inserter(resBuffer), te);
        //_signer.Sign()
        resBuffer.clear();
        te["info"]["signature"] = std::string("SIGN");
        //std::cout << te << std::endl;
        //te.print(std::cout);
        libtorrent::bencode(std::back_inserter(resBuffer), te);
        _torrent.open(QIODevice::WriteOnly | QIODevice::Truncate);
        _torrent.write(resBuffer);
        _torrent.close();
        //std::cout << resBuffer.constData();
    }
    return 0;
}

void application_update::_readTorrent()
{
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
    using namespace libtorrent;

    /*QCryptographicHash hash(QCryptographicHash::Sha1);
    QFile fd(f.absoluteFilePath());
    fd.open(QIODevice::ReadOnly);
    hash.addData(&fd);
    fd.close();
    QByteArray sha1 = hash.result();*/

    QFile fd(f.absoluteFilePath());
    fd.open(QIODevice::ReadOnly);
    _hash(&fd);
    fd.close();

    file_entry e;
    QByteArray entry_path = "bitwebsite/" + path.join("/").toUtf8();
    e.path = entry_path.constData();
    e.size = f.size();
    fs.add_file(e);//, sha1.constData());

    unsigned pad_size = _piece_size - (e.size % _piece_size);
    if(pad_size == _piece_size || pad_size == 0) return;

    /*hash.reset();
    hash.addData(QByteArray(pad_size, 0));
    sha1 = hash.result();*/

    _hash(QByteArray(pad_size, 0));

    QByteArray entry_pad_path = "bitwebsite/.padding/" + path.join("/").toUtf8();
    e.path = entry_pad_path.constData();
    e.size = pad_size;
    e.pad_file = true;
    fs.add_file(e);//, sha1.constData());
}

void application_update::_pad(libtorrent::file_storage &fs, unsigned piece_size)
{/*
    using namespace libtorrent;

    for (std::vector<internal_file_entry>::iterator i = fs.begin(); i != fs.end(); ++i) {
        unsigned pad_size = piece_size - (i->size % piece_size);
        if(pad_size == piece_size || pad_size == 0) continue;

        fs.add_file();
        internal_file_entry e;
        int cur_index = file_index(*i);
        i = m_files.insert(i, e);
        i->size = pad_size;
        i->offset = off;
        char name[30];
        snprintf(name, sizeof(name), ".____padding_file/%d", padding_file);
        std::string path = combine_path(m_name, name);
        i->set_name(path.c_str());
        i->pad_file = true;
        off += pad_size;
        ++padding_file;

        if (int(m_mtime.size()) > cur_index)
            m_mtime.insert(m_mtime.begin() + cur_index, 0);

        if (int(m_file_hashes.size()) > cur_index)
            m_file_hashes.insert(m_file_hashes.begin()
                + cur_index, (char const*)NULL);

        if (int(m_file_base.size()) > cur_index)
            m_file_base.insert(m_file_base.begin() + cur_index, 0);

        // skip the pad file we just added and point
        // at the current file again
        ++i;
    }*/
}

void application_update::_hash(const QByteArray &data)
{
    const char *d = data.constData();
    unsigned size = data.size();
    while(size > 0) {
        unsigned chunk_size = qMin(_piece_size - _hasher_size, size);
        _hasher_size += chunk_size;
        _hasher.update(d, chunk_size);
        d = &d[chunk_size];
        size -= chunk_size;
        if(_hasher_size == _piece_size) {
            _hashs.append(_hasher.final());
            _hasher.reset();
            _hasher_size = 0;
        }
    }
}

void application_update::_hash(QIODevice *f)
{
    while(!f->atEnd()) {
        _hash(f->read(_piece_size - _hasher_size));
    }
}

} // namespace bitweb
