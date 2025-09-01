#ifndef REMOTECOMMANDS_H
#define REMOTECOMMANDS_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "core/application.h"
#include "remotecontroller/remoteplaylist.h"
#include "remotecontroller/remotebasiccommands.h"

class RemoteCommands : public QObject
{
  Q_OBJECT

  // List of commands that are not in the basic command list. Requires more processing.
  QList<QString> otherCommands{
    QStringLiteral("radio"),
    QStringLiteral("queue"),
    QStringLiteral("collection"),
    QStringLiteral("context"),
    QStringLiteral("remove-current-song"),
  };

  Application   *app_;
  RemotePlaylist playlist;
  RemoteBasicCommands basicCommands;

public:
  explicit RemoteCommands(Application *app, QObject *parent);
  ~RemoteCommands() = default;

  // QString RemoteCommands::processCommand(const QString& command, const QStringList &args)
  void processCommand(const QString& command, const QStringList& args);

public Q_SLOTS:
  void processLine(const QString& line);

Q_SIGNALS:
  void forwardToPlayer(const QString& command, const QStringList& args);


};

#endif // REMOTECOMMANDS_H
