#ifndef LIBTORRENT_EXTENSIONS_BW_VERSION_PEER_PLUGIN_H
#define LIBTORRENT_EXTENSIONS_BW_VERSION_PEER_PLUGIN_H

#include <libtorrent/extensions.hpp>

#include "bw_version_plugin.h"

/*
 * - Tell during handshake the version information (if we have any)
 * - Get information about the version information of the other peer when we receive the extension handshake
 * - When we detect on our side a new version (through the alert system), send a bW_version message
 * - When the other peer tells us of a new version through a bW_version, store it (and don't send it back to this peer). use the alert system
 */

namespace libtorrent {
namespace extensions {

class bw_version_plugin;
class bw_version_torrent_plugin;

class bw_version_peer_plugin : public libtorrent::peer_plugin, public bw_version_observee
{
public:
    bw_version_peer_plugin(bw_version_plugin &plugin, bw_version_torrent_plugin &parent, peer_connection &conn);
    virtual ~bw_version_peer_plugin();

    const char *type() const;
    void add_handshake(entry &);
    bool on_extension_handshake(const lazy_entry &);
    bool on_extended(int, int, buffer::const_interval);

    void on_new_version_detected();

private:
    bw_version_plugin         &_plugin;
    bw_version_torrent_plugin &_parent;
    peer_connection           &_connection;

    int _extid;
};

} // namespace extensions
} // namespace libtorrent

#endif // LIBTORRENT_EXTENSIONS_BW_VERSION_PEER_PLUGIN_H
