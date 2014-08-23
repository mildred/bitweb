#ifndef LIBTORRENT_EXTENSIONS_BW_VERSION_TORRENT_PLUGIN_H
#define LIBTORRENT_EXTENSIONS_BW_VERSION_TORRENT_PLUGIN_H

#include <libtorrent/extensions.hpp>

#include "bw_version_plugin.h"

/*
 * - Check every so often (15m) if there is a new version (use the alert system to notify ourselves)
 * - Publish every hour the version of this torrent (if it is not outdated)
 */

namespace libtorrent {
namespace extensions {

class bw_version_plugin;

class bw_version_torrent_plugin : public libtorrent::torrent_plugin, public bw_version_observee
{
public:
    bw_version_torrent_plugin(bw_version_plugin &parent, torrent &t);
    virtual ~bw_version_torrent_plugin();

    boost::shared_ptr<peer_plugin> new_connection(peer_connection *);
    void tick();
    void on_new_version_detected();

private:
    bw_version_plugin &_parent;
    torrent           &_torrent;
};

} // namespace extensions
} // namespace libtorrent

#endif // LIBTORRENT_EXTENSIONS_BW_VERSION_TORRENT_PLUGIN_H
