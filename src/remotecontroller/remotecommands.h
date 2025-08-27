#ifndef REMOTECOMMANDS_H
#define REMOTECOMMANDS_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

enum BasicCommands
{
  PLAY,
  PLAY_PAUSE,
  PAUSE,
  STOP,
  STOP_AFTER_CURRENT,
  PREVIOUS,
  NEXT,
  VOLUME,
  VOLUME_UP,
  VOLUME_DOWN,
  VOLUME_INCREASE_BY,
  VOLUME_DECREASE_BY,
  SEEK_TO,
  SEEK_BY,
  RESTART_OR_PREVIOUS,
  PLAY_TRACK
};

class RemoteCommands : public QObject
{
  Q_OBJECT

  BasicCommands commands;

  QString processCommand(const QString& command);

public:
  explicit RemoteCommands();

public Q_SLOTS:
  void processLine(const QString& line);

Q_SIGNALS:
  void forwardToPlayer(const QString& command, const QStringList& args);

};

#endif // REMOTECOMMANDS_H
