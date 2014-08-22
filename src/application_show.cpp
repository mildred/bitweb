#include <iostream>

#include <QDebug>

#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/lazy_entry.hpp>

#include <cryptopp/rsa.h>

#include "application_show.h"

namespace bitweb {

application_show::application_show(QString torrent, QObject *parent) :
    QObject(parent),
    _torrent(torrent)
{
}

int application_show::exec()
{
    _torrent.open(QIODevice::ReadOnly);
    QByteArray fileContent = _torrent.readAll();
    const char *buf = fileContent;
    _torrent.close();

    libtorrent::lazy_entry e;
    libtorrent::error_code ec;
    int ret = libtorrent::lazy_bdecode(&buf[0], &buf[0] + fileContent.size(), e, ec);
    if (ret != 0) {
        std::cerr << "invalid bencoding: " << ec.message() << std::endl;
        return 1;
    }

    libtorrent::torrent_info t(e);
    QByteArray infoHashHex = QByteArray(t.info_hash().to_string().c_str(), 20).toHex();
    std::cout << "info hash: " << std::string(infoHashHex.constData(), infoHashHex.size()) << std::endl;
    std::cout << "torrent name: " << t.name() << std::endl;

    libtorrent::lazy_entry *info = e.dict_find("info");
    if(info) {
        libtorrent::lazy_entry *pubkey = info->dict_find("public key");
        libtorrent::lazy_entry *signature = info->dict_find("signature");
        if(pubkey && signature && pubkey->type() == libtorrent::lazy_entry::string_t && signature->type() == libtorrent::lazy_entry::string_t) {
            try {
                CryptoPP::ByteQueue bytes;
                bytes.Put((const byte*) pubkey->string_cstr(), pubkey->string_length());
                bytes.MessageEnd();
                _verifier.AccessKey().Load(bytes);
                //_verifier.AccessPublicKey().Load(bytes);

                QByteArray signatureBuffer;
                libtorrent::entry info2;
                info2 = *info;
                info2.dict().erase("signature");
                libtorrent::bencode(std::back_inserter(signatureBuffer), info2);

                bool res = _verifier.VerifyMessage(
                            (const byte*) signatureBuffer.constData(), signatureBuffer.size(),
                            (const byte*) signature->string_cstr(), signature->string_length());

                if(res) {
                    std::cout << "signature: passed" << std::endl;
                } else {
                    std::cout << "signature: failed" << std::endl;
                }
            } catch(CryptoPP::BERDecodeErr &e) {
                std::cout << "signature: (could not read public key)" << std::endl;
            }
        } else {
            std::cout << "signature: not found" << std::endl;
        }
    } else {
        std::cout << "warning: no info dict" << std::endl;
    }

    std::cout << libtorrent::print_entry(e) << std::endl;

    const libtorrent::file_storage &files = t.files();
    int index = 0;
    for (const libtorrent::internal_file_entry &f : files) {
        if(f.pad_file) continue;
        int first = t.map_file(index, 0, 1).piece;
        int last = t.map_file(index, f.size - 1, 1).piece;
        std::cout << index << ": " << files.file_path(index) << std::endl;
        std::cout << "\tsize: " << f.size << std::endl;
        std::cout << "\tpiece range: "<< first << ".." << last << std::endl;
        index++;
    }

    return 0;
}

} // namespace bitweb
