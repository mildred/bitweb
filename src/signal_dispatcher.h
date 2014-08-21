#ifndef BITWEB_SIGNAL_DISPATCHER_H
#define BITWEB_SIGNAL_DISPATCHER_H

#include <signal.h>
#include <functional>
#include <QSocketNotifier>

template<int signum>
class signal_dispatcher
{

public:
    signal_dispatcher(std::function<void()> handler, std::function<void(int)> unresponsive_handler = unresponsive_nop);

    static void signalHandler(int sig);

    static void unresponsive_nop(int);

private:
    int *makeSigFd();
    static int sigFd[2];
    static std::function<void(int)> _unresponsive_handler;
    static int pending;

    QSocketNotifier sn;
};

#include "signal_dispatcher.cpp"

#endif // BITWEB_SIGNAL_DISPATCHER_H
