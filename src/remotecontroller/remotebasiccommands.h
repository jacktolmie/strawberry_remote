#ifndef REMOTEBASICCOMMANDS_H
#define REMOTEBASICCOMMANDS_H

#include <QMap>
#include <QObject>
#include <QJsonObject>

#include "core/application.h"

class QTcpSocket;

using BasicCmdMap = QMap<QString, std::function<void(const QStringList&)>>;

class RemoteBasicCommands: public QObject
{
  Q_OBJECT

  void createCommandMap();

  Application *app_;
  BasicCmdMap commandMap;

public:
  explicit RemoteBasicCommands(Application *app);
  ~RemoteBasicCommands() = default;

  BasicCmdMap& sendCommandMap();

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

public Q_SLOTS:
  void volumeChanged(const uint volume);
};

#endif // REMOTEBASICCOMMANDS_H
