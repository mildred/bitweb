#include <QEventLoop>
#include <QDir>
#include <QDebug>
#include <QStandardPaths>
#include <QTimer>

#include <libtorrent/entry.hpp>
#include <libtorrent/peer_id.hpp>
#include <libtorrent/alert.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/lazy_entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/add_torrent_params.hpp>

#include "torrent_server.h"

#define unless(x) if(!(x))

namespace bitweb {

torrent_server::torrent_server(QObject *parent) :
    QObject(parent),
    _session(new libtorrent::session(
                 libtorrent::fingerprint("bW", LIBTORRENT_VERSION_MAJOR, LIBTORRENT_VERSION_MINOR, 0, 0),
                 std::make_pair(6881, 6889))),
    _debug(false)
{
    _session->set_alert_mask(libtorrent::alert::all_categories);
    _session->set_alert_dispatch([this](std::auto_ptr<libtorrent::alert> a){
        //qDebug() << "libtorrent" << a->message().c_str();


        libtorrent::read_piece_alert *rap = dynamic_cast<libtorrent::read_piece_alert*>(a.get());
        if(rap && rap->size == 0) {
            abort();
        }


        /*
        if(rap) {
            QByteArray h1(rap->handle.info_hash().to_string().c_str(), 20);
            QByteArray h2(f->t.info_hash().to_string().c_str(), 20);
            qDebug() << "receive piece" << rap->piece << "in range? [" << f->first_piece << f->last_piece << "]" << h1.toHex() << h2.toHex();
        }
        */

        this->metaObject()->invokeMethod(this, "onAlert", Qt::QueuedConnection, Q_ARG(void*, a.release()));
    });

    QFile stateFile(sessionStateFile());
    if(stateFile.exists()) {
        qDebug() << "restore session from" << stateFile.fileName();
        stateFile.open(QIODevice::ReadOnly);
        _session_state = stateFile.readAll();
        libtorrent::lazy_entry e;
        libtorrent::error_code ec;
        libtorrent::lazy_bdecode(_session_state.constData(),
                                 _session_state.constData() + (_session_state.size() - 1),
                                 e, ec);
        if(ec.value() == 0) _session->load_state(e);
        stateFile.close();
    }

    _session->add_dht_router(std::make_pair("router.bittorrent.com", 6881));
    _session->add_dht_router(std::make_pair("router.utorrent.com",   6881));
    _session->add_dht_router(std::make_pair("router.bitcomet.com",   6881));


    QDir cacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    QFileInfoList fil = cacheDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for(QFileInfo fi : fil) {
        QByteArray qhashinfohex = QFile::encodeName(fi.fileName());
        QByteArray qhashinfo = QByteArray::fromHex(qhashinfohex);
        if(qhashinfo.size() != 20) continue;

        libtorrent::add_torrent_params addt;
        addt.info_hash = libtorrent::sha1_hash(qhashinfo.constData());
        QByteArray torrentDir = QFile::encodeName(fi.filePath());
        addt.save_path = torrentDir.constData();
        addt.uuid = qhashinfohex.constData();

        QFile resumeFile(fi.filePath() + ".fastresume");
        std::vector<char> resumeData;
        if(resumeFile.exists()) {
            resumeFile.open(QIODevice::ReadOnly);
            QByteArray state = resumeFile.readAll();
            resumeFile.close();
            for(int i = 0; i < state.size(); ++i) resumeData.push_back(state[i]);
            addt.resume_data = &resumeData;
            qDebug() << "resume torrent from" << resumeFile.fileName();
        }

        libtorrent::error_code ec;
        _session->add_torrent(addt, ec);
        qDebug() << "start torrent from" << fi.filePath();
    }

    _session->resume();


    QTimer *timer = new QTimer(this);
    timer->setSingleShot(false);
    timer->setInterval(10000);
    timer->setTimerType(Qt::VeryCoarseTimer);
    //connect(timer, SIGNAL(timeout()), this, SLOT(syncState()));
    //connect(timer, SIGNAL(timeout()), this, SLOT(popAlerts()));
    connect(timer, SIGNAL(timeout()), this, SLOT(saveResumeData()));
    timer->start();

    print_status();
}

torrent_server::~torrent_server()
{
    saveResumeData(false);

    QByteArray buffer;
    QFile stateFile(sessionStateFile());

    libtorrent::entry e;
    _session->save_state(e);
    buffer.clear();
    libtorrent::bencode(std::back_inserter(buffer), e);

    qDebug() << "save session to" << stateFile.fileName();
    stateFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    stateFile.write(buffer);
    stateFile.close();
}

void torrent_server::saveResumeData(bool resume_session)
{
    using namespace libtorrent;

    QByteArray buffer;

    static int outstanding_resume_data; // global counter of outstanding resume data (potential race condition)
    outstanding_resume_data = 0;
    std::vector<torrent_handle> handles = _session->get_torrents();
    _session->pause();
    for (std::vector<torrent_handle>::iterator i = handles.begin();
            i != handles.end(); ++i)
    {
        torrent_handle& h = *i;
        if (!h.is_valid()) continue;
        torrent_status s = h.status();
        if (!s.has_metadata) continue;
        if (!s.need_save_resume) continue;

        h.save_resume_data();
        ++outstanding_resume_data;
    }

    while (outstanding_resume_data > 0) {
        alert_ptr a = wait_for_alert([](alert_ptr a){
            if(alert_cast<save_resume_data_failed_alert>(a.get())) --outstanding_resume_data;
            return (alert_cast<save_resume_data_alert>(a.get()) != 0);
        });

        save_resume_data_alert const* rd = alert_cast<save_resume_data_alert>(a.get());

        buffer.clear();
        bencode(std::back_inserter(buffer), *rd->resume_data);
        QFile f(QString::fromStdString(rd->handle.save_path()) + ".fastresume");
        qDebug() << "save torrent to" << f.fileName();
        f.open(QIODevice::WriteOnly | QIODevice::Truncate);
        f.write(buffer);
        f.close();
        --outstanding_resume_data;
    }

    if(resume_session) _session->resume();
    print_status();
}

void torrent_server::setDebug(bool d)
{
    _debug = d;
}

void torrent_server::prepare(QByteArray infoHash)
{
    QByteArray infoHashHex = infoHash.toHex();
    QDir cacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    cacheDir.mkpath(infoHashHex);

    libtorrent::add_torrent_params addt;
    addt.info_hash = libtorrent::sha1_hash(infoHash.constData());
    QByteArray torrentDir = QFile::encodeName(cacheDir.absoluteFilePath(infoHashHex));
    addt.save_path = torrentDir.constData();
    addt.uuid = infoHashHex.constData();

    libtorrent::error_code ec;
    libtorrent::torrent_handle t = _session->add_torrent(addt, ec);

    if(ec.value() != 0) {
        qWarning() << infoHashHex.constData() << "could not add:" << ec.message().c_str();
        return;
    }

    qDebug() << infoHashHex << "adding in" << torrentDir.constData();
    t.set_priority(t.status().priority + 1);

    print_status();
}

torrent_file::torrent_file(libtorrent::torrent_handle t, libtorrent::sha1_hash info_hash, QString path) :
    exists(false),
    index(-1),
    t(t),
    ti(t.get_torrent_info()),
    fs(ti.files()),
    file_size(0), file_offset(0), piece_length(fs.piece_length()), first_piece(0), last_piece(-1), num_pieces(0)
{
    if(ti.info("files") == 0) {
        // single file torrent
        if(path == "/") {
            index = 0;
            exists = true;
        }
    } else {
        path = QString::fromStdString(t.name()) + path;
        std::string path2 = path.toStdString();
        bool isdir = path[path.size()-1] == '/';

        int choice1 = -1, choice2 = -1;
        for(int i = 0; i < ti.num_files(); ++i) {
            if(          path2 == fs.file_path(i) + ".bitweb")    choice1 = i;
            if(!isdir && path2 == fs.file_path(i))                choice2 = i;
            if(isdir  && path2 == fs.file_path(i) + "index.html") choice2 = i;
        }

        index  = (choice1 != -1) ? choice1 : choice2;
        exists = index != -1;
    }

    if(exists) {
        file_size    = fs.file_size(index);
        file_offset  = fs.at(index).offset; // FIXME: use file_offset
        first_piece  = file_offset / piece_length;
        last_piece   = (file_offset + file_size - 1) / piece_length;
        num_pieces   = last_piece - first_piece + 1;
    }
}

torrent_server::file_ptr torrent_server::get_file(QByteArray infoHash, QString path)
{
    libtorrent::sha1_hash info_hash(infoHash.constData());
    libtorrent::torrent_handle t = _session->find_torrent(info_hash);

    while(!t.status().has_metadata)
        wait_for_torrent_alert(info_hash, libtorrent::state_changed_alert::alert_type);

    file_ptr f(new torrent_file(t, info_hash, path));
    return f;
}

bool torrent_server::stream_file(torrent_server::file_ptr f, std::function<void(QByteArray)> stream)
{
    typedef libtorrent::size_type size_type;
    if(f->index == -1) return false;

    //t.file_priority(index, 7);
    constexpr int desired_speed_bytes_ms = 10000000/1000; // 10MB/s
    for(int i = f->first_piece; i <= f->last_piece; ++i) {
        int piece_seq = i - f->first_piece;
        int deadline = (piece_seq+1) * f->piece_length / desired_speed_bytes_ms;
        if(f->t.have_piece(i)) {
            qDebug() << "have piece" << i << "now";
            f->t.read_piece(i);
        } else {
            qDebug() << "want piece" << i << "in" << deadline << "ms";
            f->t.set_piece_deadline(i, deadline,
                                 libtorrent::torrent_handle::alert_when_available);
        }
    }

    std::vector<alert_ptr> alerts(f->num_pieces, nullptr);
    int next_piece = f->first_piece;
    size_type streamed_size = 0;

    wait_for_alert([&f, &alerts, &next_piece, &streamed_size, &stream](alert_ptr a){
        libtorrent::read_piece_alert *rap = dynamic_cast<libtorrent::read_piece_alert*>(a.get());
        /*
        if(rap) {
            QByteArray h1(rap->handle.info_hash().to_string().c_str(), 20);
            QByteArray h2(f->t.info_hash().to_string().c_str(), 20);
            qDebug() << "receive piece" << rap->piece << "in range? [" << f->first_piece << f->last_piece << "]" << h1.toHex() << h2.toHex();
        }
        */
        if(rap && rap->size == 0) {
            libtorrent::torrent_status ts = rap->handle.status();
            qDebug() << ts.paused << ts.error.c_str();
        }
        unless(rap && rap->handle.info_hash() == f->t.info_hash() && rap->piece >= f->first_piece && rap->piece <= f->last_piece) return false;
        #define alerts_(n) alerts[(n) - f->first_piece]
        alerts_(rap->piece) = a;
        qDebug() << "receive piece" << rap->piece << "size" << rap->size;
        while(next_piece <= f->last_piece && alerts_(next_piece) != nullptr) {
            // do the streaming
            rap = dynamic_cast<libtorrent::read_piece_alert*>(alerts_(next_piece).get());
            int offset = 0;
            size_type size   = rap->size;
            if(next_piece == f->first_piece) {
                offset = f->file_offset % f->piece_length;
                size   = size - offset;
            } else if(next_piece == f->last_piece) {
                size = qMin(f->file_size - streamed_size, size);
            }
            stream(QByteArray(&rap->buffer.get()[offset], size));
            // FIXME: if the data has already been downloaded, the buffer may be empty (or incomplete?)
            streamed_size += size;
            alerts_(next_piece) = nullptr;
            next_piece++;
        }
        #undef alerts_
        return next_piece >= f->last_piece;
    });


    /*
    std::vector<size_type> file_progress;
    for(;;) {
        t.file_progress(file_progress, libtorrent::torrent_handle::piece_granularity);
        if(file_progress[index] == file_size) break;
        wait_for_torrent_alert(info_hash, libtorrent::state_changed_alert::alert_type);
    }

    QFile f(QDir(QString::fromStdString(t.save_path())).absoluteFilePath(QString::fromStdString(ti.files().file_path(index))));
    f.open(QIODevice::ReadOnly);
    while(!f.atEnd()) {
        stream(f.read(1024 * 16), f.atEnd());
    }
    f.close();

    //t.file_priority(index, 1);
    */
    f->t.set_priority(f->t.status().priority - 1);

    return true;
}

void torrent_server::print_status()
{
    std::vector<libtorrent::torrent_status> ret;
    auto truth = [](libtorrent::torrent_status const&){return true;};
    _session->get_torrent_status(&ret, truth);
    qDebug() << "status:" << ret.size() << "torrents";
    for(libtorrent::torrent_status &st : ret) {
        std::string infoHashStr = st.info_hash.to_string();
        QByteArray infoHash = QByteArray(infoHashStr.c_str()).toHex();
        QString state;
        switch(st.state) {
        case libtorrent::torrent_status::queued_for_checking:   state = tr("Queued for checking"); break;
        case libtorrent::torrent_status::checking_files:        state = tr("Checking files"); break;
        case libtorrent::torrent_status::downloading_metadata:  state = tr("Downloading metadata"); break;
        case libtorrent::torrent_status::downloading:           state = tr("Downloading"); break;
        case libtorrent::torrent_status::finished:              state = tr("Finished"); break;
        case libtorrent::torrent_status::seeding:               state = tr("Seeding"); break;
        case libtorrent::torrent_status::allocating:            state = tr("Allocating"); break;
        case libtorrent::torrent_status::checking_resume_data:  state = tr("Checking resume data"); break;
        default:                                                state = tr("unknown status %1").arg(st.state); break;
        }
        qDebug() << infoHash << state;
    }
}

torrent_server::alert_ptr torrent_server::wait_for_alert(std::function<bool(torrent_server::alert_ptr)> matcher)
{
    QEventLoop l;
    alert_ptr saved_alert;
    auto connection = connect(this, &torrent_server::alert, [&saved_alert, &l, &matcher](alert_ptr a){
        if(matcher(a)) {
            l.quit();
            saved_alert = a;
        }
    });
    l.exec();
    disconnect(connection);
    return saved_alert;
}

torrent_server::alert_ptr torrent_server::wait_for_alert(int alert_type)
{
    return wait_for_alert([alert_type](alert_ptr a){
        return (alert_type == -1 || a->type() == alert_type);
    });
}

torrent_server::alert_ptr torrent_server::wait_for_torrent_alert(libtorrent::sha1_hash info_hash, int alert_type)
{
    return wait_for_alert([info_hash, alert_type](alert_ptr a){
        libtorrent::torrent_alert *ta = dynamic_cast<libtorrent::torrent_alert *>(a.get());
        return (alert_type == -1 || a->type() == alert_type) && ta && ta->handle.info_hash() == info_hash;
    });
}

void torrent_server::popAlerts()
{
    std::deque<libtorrent::alert*> alerts;
    _session->pop_alerts(&alerts);
    for(libtorrent::alert *a : alerts) {
        qDebug() << "libtorrent" << a->message().c_str();
        alert_ptr a_(a);
        emit alert(a_);
    }
}

void torrent_server::onAlert(void *_a)
{
    libtorrent::alert *a = reinterpret_cast<libtorrent::alert*>(_a);
    // https://code.google.com/p/libtorrent/issues/detail?id=664
    //if(a->type() == libtorrent::add_torrent_alert::alert_type) {
    //    libtorrent::add_torrent_alert *aa = dynamic_cast<libtorrent::add_torrent_alert*>(a);
    //    qDebug() << "libtorrent add torrent" << aa->params.info_hash.to_string().c_str();
    //} else {
    std::string typeName = std::string("libtorrent::") + a->what();
    if(_debug) qDebug() << typeName.c_str() << a->message().c_str();
    //}
    alert_ptr a_(a);
    emit alert(a_);
}

QString torrent_server::sessionStateFile()
{
    QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    dataDir.mkpath(".");
    return dataDir.absoluteFilePath("session.state");
}

} // namespace bitweb
