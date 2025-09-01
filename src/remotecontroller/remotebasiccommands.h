#ifndef REMOTEBASICCOMMANDS_H
#define REMOTEBASICCOMMANDS_H

#include <QDebug>
#include <QMap>
#include <QObject>
#include <QStringList>

#include "core/application.h"

class RemoteBasicCommands
{
  void createCommandMap();

  Application *app_;
  QMap<QString, std::function<void(const QStringList&)>> commandMap;

public:
  explicit RemoteBasicCommands(Application *app);
  ~RemoteBasicCommands() = default;

  bool sendCommand(const QString &command, const QStringList &args);
};

#endif // REMOTEBASICCOMMANDS_H
