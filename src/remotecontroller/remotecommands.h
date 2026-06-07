#ifndef REMOTECOMMANDS_H
#define REMOTECOMMANDS_H

#include <QObject>

#include "core/application.h"
#include "remotecontroller/remoteplaylist.h"
#include "remotecontroller/remotebasiccommands.h"

class QTcpSocket;


class RemoteCommands : public QObject
{
  Q_OBJECT

  const Application         *app_;

  const RemotePlaylist      remotePlaylist;
  const RemoteBasicCommands basicCommands;

  BasicCmdMap               basicCmdMap;
  PlaylistCmdMap            playlistCmdMap;

public:
  explicit RemoteCommands(Application *app, QObject *parent);
  ~RemoteCommands() = default;

  void processCommand(const QString& command, const QStringList& args);
  const RemotePlaylist& getRemotePlaylist() const { return remotePlaylist; }

public Q_SLOTS:
  void getResponse(const QJsonObject& response);
  void processLine(const QString& line);

Q_SIGNALS:
  void forwardToPlayer(const QString& command, const QStringList& args);
  void sendReponse(const QJsonObject& response);
};

#endif // REMOTECOMMANDS_H
