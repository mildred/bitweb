#include <iostream>

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

    //std::cout << e << std::endl;

    libtorrent::torrent_info t(e);
    const libtorrent::file_storage &files = t.files();
    int index = 0;
    for (const libtorrent::internal_file_entry &f : files) {
        int first = t.map_file(index, 0, 1).piece;
        int last = t.map_file(index, f.size - 1, 1).piece;
        std::cout << index << ": " << files.file_path(index) << std::endl;
        std::cout << "\tsize: " << f.size << std::endl;
        std::cout << "\tpiece range: "<< first << ".." << last << std::endl;
        index++;
    }

#if 0

    using namespace libtorrent;

    torrent_info t(e);

    // print info about torrent
    std::cout << "\n\n----- torrent file info -----\n\n";
    std::cout << "nodes:\n";
    typedef std::vector<std::pair<std::string, int> > node_vec;
    node_vec const& nodes = t.nodes();
    for (node_vec::const_iterator i = nodes.begin(), end(nodes.end());
           i != end; ++i)
    {
           std::cout << i->first << ":" << i->second << "\n";
    }
    std::cout << "trackers:\n";
    for (std::vector<announce_entry>::const_iterator i = t.trackers().begin();
           i != t.trackers().end(); ++i)
    {
           std::cout << i->tier << ": " << i->url << "\n";
    }

    std::cout << "number of pieces: " << t.num_pieces() << "\n";
    std::cout << "piece length: " << t.piece_length() << "\n";
    std::cout << "info hash: " << t.info_hash() << "\n";
    std::cout << "comment: " << t.comment() << "\n";
    std::cout << "created by: " << t.creator() << "\n";
    std::cout << "files:\n";
    int index = 0;
    for (torrent_info::file_iterator i = t.begin_files();
           i != t.end_files(); ++i, ++index)
    {
           int first = t.map_file(index, 0, 1).piece;
           int last = t.map_file(index, i->size - 1, 1).piece;
           std::cout << "  " << std::setw(11) << i->size
                   << " " << i->path.string() << "[ " << first << ", "
                   << last << " ]\n";
    }
#endif

    return 0;
}

} // namespace bitweb
