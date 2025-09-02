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

  Application   *app_;
  RemotePlaylist playlist;
  RemoteBasicCommands basicCommands;

public:
  explicit RemoteCommands(Application *app, QObject *parent);
  ~RemoteCommands() = default;

  // QString RemoteCommands::processCommand(const QString& command, const QStringList &args)
  void processCommand(const QString& command, const QStringList& args);

public Q_SLOTS:
  void processLine(QTcpSocket* clientSocket, const QString& line);

Q_SIGNALS:
  void forwardToPlayer(const QString& command, const QStringList& args);
  void sendReponse(QTcpSocket* clientSocket, const QJsonObject& response);

private Q_SLOTS:
  void getAllPlaylists(const QJsonObject& playlists);
  void getCurrentPlaylist(const QJsonObject& playlist);

};

#endif // REMOTECOMMANDS_H
