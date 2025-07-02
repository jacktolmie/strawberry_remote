#include "remotecontroller.h"


RemoteController::RemoteController(QObject *parent)
    : QObject{parent}
{

}

// void RemoteController::ExitFinished(){}

void RemoteController::Exit(){

  Q_EMIT ExitFinished();
}

void RemoteController::ExitReceived(){

}
