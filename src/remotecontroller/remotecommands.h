#ifndef REMOTECOMMANDS_H
#define REMOTECOMMANDS_H

#include <QObject>

class RemoteCommands : public QObject
{
     Q_OBJECT
public:
    explicit RemoteCommands(QObject *parent = nullptr);

// signals:

};

#endif // REMOTECOMMANDS_H
