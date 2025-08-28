#ifndef REMOTECOMMANDS_H
#define REMOTECOMMANDS_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "core/application.h"

class RemoteCommands : public QObject
{
  Q_OBJECT

  // List of basic commands to be sent to MainWindow::handleRemoteCommand
  QList<QString> commands{
    QStringLiteral("play"),
    QStringLiteral("play-pause"),
    QStringLiteral("pause"),
    QStringLiteral("stop"),
    QStringLiteral("stop-after-current"),
    QStringLiteral("next"),
    QStringLiteral("previous"),
    QStringLiteral("restart-or-previous"),
    QStringLiteral("volume"),
    QStringLiteral("volume-up"),
    QStringLiteral("volume-down"),
    QStringLiteral("volume-increase-by"),
    QStringLiteral("volume-decrease-by"),
    QStringLiteral("seek-to"),
    QStringLiteral("seek-by")

  };
  Application   *app_;

  QString processCommand(const QString& command);

public:
  explicit RemoteCommands(Application *app, QObject *parent);

public Q_SLOTS:
  void processLine(const QString& line);

Q_SIGNALS:
  void forwardToPlayer(const QString& command, const QStringList& args);

};

#endif // REMOTECOMMANDS_H
