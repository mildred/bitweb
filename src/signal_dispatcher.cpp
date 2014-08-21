#include <unistd.h>
#include <sys/socket.h>
#include "signal_dispatcher.h"

template<int signum>
signal_dispatcher<signum>::signal_dispatcher(std::function<void()> handler, std::function<void (int)> unresponsive_handler) :
    sn(makeSigFd()[1], QSocketNotifier::Read)
{
    _unresponsive_handler = unresponsive_handler;
    pending = 0;
    QObject::connect(&sn, &QSocketNotifier::activated, [this, handler](){
        sn.setEnabled(false);
        int data;
        ::read(sigFd[1], &data, sizeof(data));
        pending--;

        handler();

        sn.setEnabled(true);
    });

    struct sigaction act;

    act.sa_handler = signal_dispatcher::signalHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_flags |= SA_RESTART;

    if (sigaction(signum, &act, 0) > 0) {
        qFatal("Couldn't set signal handler %d", signum);
    }
}

template<int signum>
void signal_dispatcher<signum>::signalHandler(int sig)
{
    _unresponsive_handler(pending);
    pending++;
    ::write(sigFd[0], &sig, sizeof(sig));
}

template<int signum>
void signal_dispatcher<signum>::unresponsive_nop(int){}

template<int signum>
int *signal_dispatcher<signum>::makeSigFd()
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigFd))
       qFatal("Couldn't create socketpair");
    return sigFd;
}

template<int signum>
int signal_dispatcher<signum>::sigFd[2];

template<int signum>
int signal_dispatcher<signum>::pending = 0;

template<int signum>
std::function<void(int)> signal_dispatcher<signum>::_unresponsive_handler;
