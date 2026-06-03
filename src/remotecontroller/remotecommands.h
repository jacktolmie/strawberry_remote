#ifndef REMOTECOMMANDS_H
#define REMOTECOMMANDS_H

#include <QObject>

#include "core/application.h"
#include "remotecontroller/remoteplaylist.h"
#include "remotecontroller/remotebasiccommands.h"
#include "remotecontroller/remoteguivalues.h"

class QTcpSocket;
class RemoteGuiValues;


class RemoteCommands : public QObject
{
  Q_OBJECT

  const Application         *app_;

  const RemotePlaylist      remotePlaylist;
  const RemoteBasicCommands basicCommands;
  const RemoteGuiValues     *values;

  BasicCmdMap               basicCmdMap;
  PlaylistCmdMap            playlistCmdMap;


public:
  explicit RemoteCommands(Application *app, QObject *parent);
  ~RemoteCommands() = default;

  void processCommand(const QString& command, const QStringList& args);

public Q_SLOTS:
  // void sendGuiUpdate(const QTcpSocket *client, QJsonObject updates);
  void sendGuiUpdate();
  void getResponse(const QJsonObject& response);
  void processLine(const QString& line);
  // void processLine(QTcpSocket* clientSocket, const QString& line);

Q_SIGNALS:
  void forwardToPlayer(const QString& command, const QStringList& args);
  void sendReponse(const QJsonObject& response);

private Q_SLOTS:


};

#endif // REMOTECOMMANDS_H
