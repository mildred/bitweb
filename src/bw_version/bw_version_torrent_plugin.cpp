#include "bw_version_torrent_plugin.h"
#include "bw_version_peer_plugin.h"
#include "bw_version_plugin.h"

namespace libtorrent {
namespace extensions {

bw_version_torrent_plugin::bw_version_torrent_plugin(bw_version_plugin &parent, torrent &t) :
    _parent(parent),
    _torrent(t)
{
    _parent.register_new_version_callback(this);
}

bw_version_torrent_plugin::~bw_version_torrent_plugin()
{
    _parent.unregister_new_version_callback(this);
}

boost::shared_ptr<peer_plugin> bw_version_torrent_plugin::new_connection(peer_connection *conn)
{
    boost::shared_ptr<peer_plugin> ptr(new libtorrent::extensions::bw_version_peer_plugin(_parent, *this, *conn));
}

void bw_version_torrent_plugin::tick()
{
    // TODO: Don't do that if there is another torrent on our side that is a more recent version
    if(true/* we are an old version */) return;

    // TODO: Every 15 minutes check for a new version of each site
    if(false) {
        _parent.emit_new_version();
    }

    // TODO: Every hour, publish the version information
}

void bw_version_torrent_plugin::on_new_version_detected()
{
    if(true /* TODO: we are not interested*/) return;

    // TODO: Check to see if we are out of date (do it here or not?)
}

} // namespace extensions
} // namespace libtorrent
