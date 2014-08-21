#include <typeinfo>
#include <iostream>
#include "application.h"
#include "stacktrace.h"
#include "deathhandler/death_handler.h"

int main(int argc, char *argv[]){
    Debug::DeathHandler dh;
    dh.set_frames_count(100);
    bitweb::application app(argc, argv);
    if(!app.parseArguments()) return 1;
    return app.exec();
}
