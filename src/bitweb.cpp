#include <QCommandLineOption>
#include <QCommandLineParser>

int main(int argc, char *argv[]){
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("bitweb");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("BitTorrent Web Proxy");
    parser.addHelpOption();
    parser.addVersionOption();

    // An option with a value
    QCommandLineOption portOption(QStringList() << "p" << "Port",
            QCoreApplication::translate("main", "SOCKS4A proxy listen to tcp <port>"),
            QCoreApplication::translate("main", "port"));
    parser.addOption(portOption);

    // Process the actual command line arguments given by the user
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    // source is args.at(0), destination is args.at(1)

    int port = parser.value(portOption).toInt();
}
