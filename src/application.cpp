#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>

#include <libtorrent/ed25519.hpp>
#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>

#include "qstdstream.h"
#include "application.h"
#include "application_server.h"
#include "application_show.h"
#include "application_update.h"

namespace bitweb {

static void quit(){
    fprintf(stderr, "Stop event loop...\n");
    QCoreApplication::exit(0);
}

static void force_quit_int(int num){
    if(num == 0) {
        fprintf(stderr, "Received interrupt, quitting gracefully. Hit Ctrl-C again to force operation.\n", num);
        return;
    } else if(num < 5) {
        fprintf(stderr, "Received interrupt, %d pending. Waiting %d more to force operation.\n", num, 5-num);
    } else {
        fprintf(stderr, "Received interrupt, %d pending. Force operation.\n", num);
        abort();
    };
}

static void force_quit_term_alarm(int sig){
    fprintf(stderr, "Force quit (alarm)...\n");
    abort();
}

static void force_quit_term(int num){
    if(num == 0){
        fprintf(stderr, "Terminating...\n");
        signal(SIGALRM, &force_quit_term_alarm);
        alarm(1000);
    } else {
        fprintf(stderr, "Force quit (%d)...\n", num);
        abort();
    }
}

application::application(int &argc, char **argv) :
    QCoreApplication(argc, argv),
    run(false),
    sigint(&quit, &force_quit_int),
    sigterm(&quit, &force_quit_term),
    server(nullptr),
    show(nullptr),
    update(nullptr)
{
    QCoreApplication::setApplicationName("bitweb");
    QCoreApplication::setApplicationVersion("1.0");
}

bool application::parseArguments()
{
#if 0
    // My dream parser
    cmd::parser parser("BitWeb")
            .foldoptions(true); // true:  -Cph PATH HEADER:VAL
                                // false: -CpPATH -hHEADER:VAL

    cmd::group   general(parser, "General Options").required();
    cmd::flag    help(general)       .names('h', "--help")  .desc(tr("Help"));
    cmd::flag    daemon(general)     .names('D', "--daemon").desc(tr("Run as a daemon with a SOCKS proxy"));
    cmd::flag    create(general)     .names('C', "--create").desc(tr("Create a torrent file"));
    cmd::flag    update(general)     .names('U', "--update").desc(tr("Update a torrent"));
    cmd::flag    show(general)       .names('S', "--show")  .desc(tr("Show the contents of a torrent"));

    cmd::group   daemongoup(parser, "Daemon Options (-D)").disable();
    cmd::option  port(daemongroup)   .names('p', "--port")  .arg("port", "8878")  .desc(tr("SOCKS port number"));

    cmd::group   torrentgroup(parser, "Torrent Options (-C, -U, -S)").disable();
    cmd::option  path(torrentgroup)  .names('p', "--path")  .arg("path")          .desc(tr("Specify path in torrent file"));
    cmd::option  header(torrentgroup).names('h', "--header").arg("header[:value]").desc(tr("Specify header for a path in torrent file"));
    cmd::option  key(torrentgroup)   .names('k', "--key")   .arg("keyfile")       .desc(tr("Specify private key in PEM format for torrent (created if it doesn't exists)"));
    cmd::options parent(torrentgroup).names('P', "--parent").args("info_hash")    .desc(tr("Specify parent infohash"));
    cmd::arg     directory(torrentgroup)                    .arg("dir")           .desc(tr("Directory to create the torrent from")).required().disable();

    help.run([](){
        std::cout << parser.help_message(true); // true: show all even disable, false: don't show disabled
        parser.stop();
    });

    daemon.run([](){
        help.disable();
        daemongroup.enable();
    });

    create.run([](){
        help.disable();
        torrentgroup.enable();
        directory.enable();
    });

    update.run([](){
        help.disable();
        torrentgroup.enable();
    });

    show.run([](){
        help.disable();
        torrentgroup.enable();
        key.disable();
        parent.disable();
    });

    parser.parse(qApp().arguments())
#endif

    QCommandLineOption daemonOption(
                QStringList() << "D" << "daemon",
                tr("Run as a daemon with a SOCKS proxy"));
    QCommandLineOption portOption(
                QStringList() << "p" << "port",
                tr("[D] SOCKS4A proxy listen to tcp <port>"),
                tr("port"),
                "8878");
    QCommandLineOption debugOption(
                QStringList() << "d" << "debug",
                tr("Enable debug mode"));

    QCommandLineOption genKeyOption(
                QStringList() << "K" << "generate-key",
                tr("Generate a private/public key pair."));

    QCommandLineOption createOption(
                QStringList() << "C" << "create",
                tr("Create a torrent file"));
    QCommandLineOption updateOption(
                QStringList() << "U" << "update",
                tr("Update a torrent"));
    QCommandLineOption showOption(
                QStringList() << "S" << "show",
                tr("Show the contents of a torrent"));
    QCommandLineOption torrentOption(
                QStringList() << "t" << "torrent",
                tr("[CUS] Specify torrent file"),
                tr("file"));
    QCommandLineOption pathOption(
                QStringList() << "p" << "path",
                tr("[CUS] Specify path in torrent file"),
                tr("path"));
    QCommandLineOption headerOption(
                QStringList() << "H" << "header",
                tr("[CUS] Specify header for a path in torrent file"),
                tr("header[:value]"));
    QCommandLineOption keyOption(
                QStringList() << "k" << "key",
                tr("[CU] Specify private key in PEM format for torrent (created if it doesn't exists)"),
                tr("keyfile"));
    QCommandLineOption parentOption(
                QStringList() << "P" << "parent",
                tr("[CU] Specify parent infohash"),
                tr("infohash[,infohash...]"));

    QCommandLineParser parser;
    parser.setApplicationDescription("BitTorrent Web Proxy");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOption(daemonOption);
    parser.addOption(portOption);
    parser.addOption(debugOption);

    parser.addOption(genKeyOption);

    parser.addOption(createOption);
    parser.addOption(updateOption);
    parser.addOption(showOption);
    parser.addOption(torrentOption);
    parser.addOption(pathOption);
    parser.addOption(headerOption);
    parser.addOption(keyOption);
    parser.addOption(parentOption);

    // Process the actual command line arguments given by the user
    parser.process(*this);
    QStringList args = parser.positionalArguments();

    int port = parser.value(portOption).toInt();

    if(parser.isSet(genKeyOption)){
        QString fname = parser.isSet(keyOption) ? parser.value(keyOption) : "%.key";
        generateKeyPair(fname);
    }

    if(parser.isSet(daemonOption)) {
        server = new application_server(port, this);
        server->setDebug(parser.isSet(debugOption));
        run = true;
    } else if(parser.isSet(showOption)) {
        show = new application_show(parser.value(torrentOption), this);
    } else if(parser.isSet(updateOption)) {
        update = new application_update(parser.value(torrentOption), false, this);
        if(parser.isSet(keyOption))
            if(!update->setKeyFile(parser.value(keyOption))) return false;
        if(!update->open()) return false;
        if(parser.isSet(pathOption) && parser.isSet(headerOption)) {
            QString path = parser.value(pathOption);
            QString headerval = parser.value(headerOption);
            int idx = headerval.indexOf(':');
            if(idx < 0) {
                QFile err;
                err.open(stderr, QIODevice::WriteOnly);
                err.write("Header value should be HEADER:VALUE\n");
                err.close();
            } else {
                update->setHeader(path, headerval.left(idx), headerval.right(headerval.size() - idx - 1));
            }
        }
    } else if(parser.isSet(createOption)) {
        update = new application_update(parser.value(torrentOption), true, this);
        update->setCreationDirectory(args.first());
        if(parser.isSet(keyOption))
            if(!update->setKeyFile(parser.value(keyOption))) return false;
        if(!update->open()) return false;
    } else if(!parser.isSet(genKeyOption)) {
        qDebug() << args;
        QFile err;
        err.open(stderr, QIODevice::WriteOnly);
        err.write("Expected one option in -D, -C, -S, -U\n");
        err.write(parser.helpText().toLocal8Bit());
        return false;
    }

    return true;
}

bitweb::application::~application()
{
    if(server) delete server;
    if(show)   delete show;
    if(update) delete update;
}

int application::exec()
{
    if(show)   return show->exec();
    if(update) return update->exec();
    if(!run)   return 0;
    return QCoreApplication::exec();
}

void application::generateKeyPair(QString filename)
{
    libtorrent::entry keypair;
    QByteArray buffer;
    unsigned char seed[ed25519_seed_size];
    unsigned char public_key[ed25519_public_key_size];
    unsigned char private_key[ed25519_private_key_size];

    ed25519_create_seed(seed);
    ed25519_create_keypair(public_key, private_key, seed);
    keypair["type"] = "ed25519";
    keypair["public key"] = std::string((char*) public_key, (unsigned long) ed25519_public_key_size);
    keypair["private_key"] = std::string((char*) private_key, (unsigned long) ed25519_private_key_size);
    libtorrent::bencode(std::back_inserter(buffer), keypair);

    QByteArray pub = QByteArray((char*) public_key, (unsigned long) ed25519_public_key_size).toHex();
    QFile f(filename.replace("%", pub));
    f.open(QIODevice::WriteOnly | QIODevice::Truncate);
    f.write(buffer);
    f.close();

    std::cout << "Public Key: " << pub << std::endl;
    std::cout << "File: " << f.fileName().toLocal8Bit() << std::endl;
}

} // namespace bitweb
