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

  Application         *app_;

  RemotePlaylist      playlist;
  RemoteBasicCommands basicCommands;
  RemoteGuiValues     *values;

  BasicCmdMap         basicCmdMap;
  PlaylistCmdMap      playlistCmdMap;


public:
  explicit RemoteCommands(Application *app, QObject *parent);
  ~RemoteCommands() = default;

  void processCommand(QTcpSocket *clientSocket, const QString& command, const QStringList& args);

public Q_SLOTS:
  void getGuiUpdate(QTcpSocket *client, QJsonObject updates);
  void getResponse(const QJsonObject& response);
  void processLine(QTcpSocket* clientSocket, const QString& line);

Q_SIGNALS:
  void forwardToPlayer(const QString& command, const QStringList& args);
  void sendReponse(const QJsonObject& response);

private Q_SLOTS:


};

#endif // REMOTECOMMANDS_H
