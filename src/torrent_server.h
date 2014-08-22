#ifndef BITWEB_TORRENT_SERVER_H
#define BITWEB_TORRENT_SERVER_H

#include <memory>
#include <functional>

#include <QObject>

#include <libtorrent/peer_id.hpp>
#include <libtorrent/torrent_info.hpp>
#include <libtorrent/torrent_handle.hpp>
#include <libtorrent/storage.hpp>

namespace libtorrent { class session;
                       class alert; }

namespace bitweb {

class torrent_server;

class torrent_file
{
public:
    typedef libtorrent::size_type size_type;

    torrent_file(libtorrent::torrent_handle t, libtorrent::sha1_hash info_hash, QString path);
    bool                          exists;
    int                           index;
    libtorrent::torrent_handle    t;
    libtorrent::torrent_info      ti;
    libtorrent::file_storage      fs;
    const libtorrent::lazy_entry *headers;
    int                           http_status;
    std::string                   http_status_message;

    size_type file_size;
    size_type file_offset;
    int       piece_length;
    int       first_piece;
    int       last_piece;
    int       num_pieces;
};

class torrent_server : public QObject
{
    friend class torrent_file;
    Q_OBJECT

public:

    typedef std::shared_ptr<libtorrent::alert> alert_ptr;
    typedef std::shared_ptr<torrent_file>      file_ptr;

    torrent_server(QObject *parent = 0);
    virtual ~torrent_server();

    void setDebug(bool d);

    void     prepare(QByteArray infoHash);
    file_ptr get_file(QByteArray infoHash, QString path);
    bool     stream_file(torrent_server::file_ptr f, std::function<void(QByteArray)> stream);

    void print_status();

    alert_ptr wait_for_alert(std::function<bool(alert_ptr)> matcher);
    alert_ptr wait_for_alert(int alert_type = -1);
    alert_ptr wait_for_torrent_alert(libtorrent::sha1_hash info_hash, int alert_type = -1);

signals:

    void alert(alert_ptr alert);

private slots:

    void saveResumeData(bool resume_session = true);
    void onAlert(void *a);

private:
    QString sessionStateFile();

    libtorrent::session *_session;
    QByteArray _session_state;
    bool _debug;
};

} // namespace bitweb

#endif // BITWEB_TORRENT_SERVER_H
