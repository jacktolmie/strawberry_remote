#ifndef REMOTECONTROLLER_H
#define REMOTECONTROLLER_H

#include <QObject>

class RemoteController : public QObject
{
  int port{5500};


     Q_OBJECT
public:
    explicit RemoteController(QObject *parent = nullptr);

    void Exit();

Q_SIGNALS:
    void ExitFinished();

private Q_SLOTS:
    void ExitReceived();

};

#endif // REMOTECONTROLLER_H
