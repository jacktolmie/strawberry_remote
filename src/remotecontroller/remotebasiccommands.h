#ifndef REMOTEBASICCOMMANDS_H
#define REMOTEBASICCOMMANDS_H

#include <QMap>
#include <QObject>

#include "core/application.h"

class QTcpSocket;

class RemoteBasicCommands
{
  void createCommandMap();

  Application *app_;
  QMap<QString, std::function<void(const QStringList&)>> commandMap;

public:
  explicit RemoteBasicCommands(Application *app);
  ~RemoteBasicCommands() = default;

  QJsonObject sendCommand(const QString &command, const QStringList &args);
};

#endif // REMOTEBASICCOMMANDS_H
