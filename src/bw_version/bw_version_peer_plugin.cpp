#include <libtorrent/entry.hpp>
#include <libtorrent/lazy_entry.hpp>

#include "bw_version_plugin.h"
#include "bw_version_peer_plugin.h"
#include "bw_version_torrent_plugin.h"

namespace libtorrent {
namespace extensions {

bw_version_peer_plugin::bw_version_peer_plugin(bw_version_plugin &plugin, bw_version_torrent_plugin &parent, peer_connection &conn) :
    _plugin(plugin),
    _parent(parent),
    _connection(conn),
    _extid(0)
{
    _plugin.register_new_version_callback(this);
}

bw_version_peer_plugin::~bw_version_peer_plugin()
{
    _plugin.unregister_new_version_callback(this);
}

const char *bw_version_peer_plugin::type() const
{
    return "bW_version";
}

void bw_version_peer_plugin::add_handshake(entry &h)
{
    // FIXME: how are extensions id allocated
    h["m"]["bW_version"] = 128;

    // TODO: Add information about the latest known version (if this is not the last)
}

bool bw_version_peer_plugin::on_extension_handshake(const lazy_entry &h)
{
    _extid = 0;

    if (h.type() != lazy_entry::dict_t) return false;
    lazy_entry const* messages = h.dict_find_dict("m");
    if (!messages) return false;

    int index = messages->dict_find_int_value("bW_version", -1);
    if (index == -1) return false;
    _extid = index;

    if(false){
        // TODO: Check if the other side included information about a newer version
        // TODO: On a new version, tell our parent
        _plugin.emit_new_version();
    }

    return true;
}

bool bw_version_peer_plugin::on_extended(int, int, buffer::const_interval)
{
    if(true /* TODO: this is not a bW_version message*/) return false;

    // TODO: If this is a bW_version message, tell our parent that we found a new version
    if(false /* TODO */) {
        _plugin.emit_new_version();
    }

    return true;
}

void bw_version_peer_plugin::on_new_version_detected()
{
    if(true /* TODO: we are not interested */) return;
    if(true /* TODO: we already know */) return;

    // TODO: Send the bW_version message to tell the other end we have a newer version
    // TODO: Don't do that if the other end already knows about it
}

} // namespace extensions
} // namespace libtorrent
