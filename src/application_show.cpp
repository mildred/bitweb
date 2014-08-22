#include <iostream>

#include <QDebug>

#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/lazy_entry.hpp>


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
