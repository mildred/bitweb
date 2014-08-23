#include <libtorrent/torrent.hpp>

#include "bw_version_torrent_plugin.h"
#include "bw_version_plugin.h"

namespace libtorrent {
namespace extensions {

bw_version_plugin::bw_version_plugin(session &session) :
    _session(session)
{}

bw_version_plugin::~bw_version_plugin()
{}

void bw_version_plugin::register_new_version_callback(bw_version_observee *cb)
{
    _callbacks.push_front(cb);
}

void bw_version_plugin::unregister_new_version_callback(bw_version_observee *cb)
{
    _callbacks.remove(cb);
}

void bw_version_plugin::emit_new_version()
{
    // TODO: Did anyone detected a new version ? If so, store it in state

    // Send alert
    for(bw_version_observee *cb : _callbacks) {
        cb->on_new_version_detected();
    }
}

void bw_version_plugin::on_tick()
{
    // TODO: On new version, add the torrent
    // TODO: When metainfo is downloaded, notify old version plugin that it should not check/advertise versions, someone else will do it instead

}

void bw_version_plugin::save_state(entry &) const
{
    // TODO: Store the latest known version of each site (=<pubkey+id>)
}

void bw_version_plugin::load_state(const lazy_entry &)
{
    // TODO: Load the latest known version of each site (=<pubkey+id>)
}

boost::shared_ptr<torrent_plugin> bw_version_plugin::new_torrent(torrent *t, void *)
{
    boost::shared_ptr<torrent_plugin> ptr(new bw_version_torrent_plugin(*this, *t));
    t->add_extension(ptr);
}

} // namespace extensions
} // namespace libtorrent
