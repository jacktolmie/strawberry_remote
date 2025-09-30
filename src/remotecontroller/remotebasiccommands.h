#ifndef REMOTEBASICCOMMANDS_H
#define REMOTEBASICCOMMANDS_H

#include <QMap>
#include <QObject>
#include <QJsonObject>

#include "core/application.h"

class QTcpSocket;

using CommandsMap = QMap<QString, std::function<void(const QStringList&)>>;

class RemoteBasicCommands: public QObject
{
  Q_OBJECT

  void createCommandMap();

  Application *app_;
  CommandsMap commandMap;

public:
  explicit RemoteBasicCommands(Application *app);
  ~RemoteBasicCommands() = default;

  CommandsMap& sendCommandMap();

  QJsonObject checkCommand(const QString &command, const QStringList &args);

Q_SIGNALS:
  //Commands to sent to server
  void mute();
  void next();
  void pause();
  void play(const quint64 playAt);
  void playPause();
  void previous();
  void restartOrPrevious();
  void seekTo(const quint32);
  void seekBackward();
  void seekForward();
  void sendResponse(QJsonObject response);
  void stop(bool stopAfterCurrent);
  void stopAfterCurrent();
  void volume(const uint volume);
  void volumeUp();
  void volumeDown();

  // Commands received from server
  // void volumeChanged(QJsonObject volume)

public Q_SLOTS:
  void volumeChanged(const uint volume);
};

#endif // REMOTEBASICCOMMANDS_H
