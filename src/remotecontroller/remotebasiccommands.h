#ifndef REMOTEBASICCOMMANDS_H
#define REMOTEBASICCOMMANDS_H

#include <QMap>
#include <QObject>
#include <QJsonObject>

#include "core/application.h"

class QTcpSocket;

using BasicCmdMap = QMap<QString, std::function<void(const QJsonObject&)>>;

class RemoteBasicCommands: public QObject
{
  Q_OBJECT

  void createCommandMap();

  const Application *app_;
  BasicCmdMap commandMap;

  void remoteSeekTo(const QJsonObject& args);
  void commandResponse(const QString& command);
  void remoteVolume(const QJsonObject& args);
  void wrongArgsSent(const QString& error);

public:
  explicit RemoteBasicCommands(const Application *app);
  ~RemoteBasicCommands() = default;

  const BasicCmdMap &sendCommandMap() const;

Q_SIGNALS:
  //Commands recieved from remote devices.
  void mute();
  void next();
  void pause();
  void play();
  void playPause();
  void previous();
  void restartOrPrevious();
  void seekTo(const quint32);
  void seekBackward();
  void seekForward();
  void sendResponse(const QJsonObject response);
  void stop(bool stopAfterCurrent);
  void stopAfterCurrent();
  void volume(const uint volume);
  void volumeUp();
  void volumeDown();

public Q_SLOTS:
  void volumeChanged(const uint volume);
};

#endif // REMOTEBASICCOMMANDS_H
