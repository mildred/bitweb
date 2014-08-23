#ifndef LIBTORRENT_EXTENSIONS_BW_VERSION_PLUGIN_H
#define LIBTORRENT_EXTENSIONS_BW_VERSION_PLUGIN_H

#include <functional>

#include <libtorrent/session.hpp>
#include <libtorrent/extensions.hpp>

/*
 * - Store version information on quit and load version information at startup
 * - If we detect a new version, add the new version torrent. Download metainfo but no not necessarily download files.
 */

namespace libtorrent {
namespace extensions {

class bw_version_observee {
public:
    virtual void on_new_version_detected() = 0;
};

class bw_version_plugin : public libtorrent::plugin
{
public:
    bw_version_plugin(libtorrent::session &session);
    virtual ~bw_version_plugin();

    void register_new_version_callback(bw_version_observee *cb);
    void unregister_new_version_callback(bw_version_observee *cb);
    void emit_new_version();

    void on_tick();
    void save_state(entry &) const;
    void load_state(const lazy_entry &);
    boost::shared_ptr<torrent_plugin> new_torrent(torrent *, void *);

private:
    libtorrent::session &_session;

    std::list<bw_version_observee*> _callbacks;
};

} // namespace extensions
} // namespace libtorrent

#endif // LIBTORRENT_EXTENSIONS_BW_VERSION_PLUGIN_H
