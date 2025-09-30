#ifndef REMOTECOMMANDS_H
#define REMOTECOMMANDS_H

#include <QObject>

#include "core/application.h"
#include "remotecontroller/remoteplaylist.h"
#include "remotecontroller/remotebasiccommands.h"
#include "remotecontroller/remoteguivalues.h"

class QTcpSocket;
class RemoteGuiValues;

using BasicCmdMap = QMap<QString, std::function<void(const QStringList&)>>;

class RemoteCommands : public QObject
{
  Q_OBJECT

  Application   *app_;

  RemotePlaylist playlist;
  RemoteBasicCommands basicCommands;
  RemoteGuiValues *values;

  BasicCmdMap  basicCmdMap;

  // List of basic commands. Forward to basicCommands if found.
  // QList<QString> basicCommandsMap{
  //   QStringLiteral("play"),
  //   QStringLiteral("play-pause"),
  //   QStringLiteral("pause"),
  //   QStringLiteral("stop"),
  //   QStringLiteral("stop-after-current"),
  //   QStringLiteral("next"),
  //   QStringLiteral("previous"),
  //   QStringLiteral("restart-or-previous"),
  //   QStringLiteral("volume"),
  //   QStringLiteral("volume-up"),
  //   QStringLiteral("volume-down"),
  //   QStringLiteral("volume-increase-by"),
  //   QStringLiteral("volume-decrease-by"),
  //   QStringLiteral("seek-to"),
  //   QStringLiteral("seek-by"),
  //   // Delete when done with song time slider
  //   QStringLiteral("current")
  // };

public:
  explicit RemoteCommands(Application *app, QObject *parent);
  ~RemoteCommands() = default;

  void processCommand(QTcpSocket *clientSocket, const QString& command, const QStringList& args);

public Q_SLOTS:
  void processLine(QTcpSocket* clientSocket, const QString& line);
  void getGuiUpdate(QTcpSocket *client, QJsonObject updates);

Q_SIGNALS:
  void forwardToPlayer(const QString& command, const QStringList& args);
  // void sendReponse(QTcpSocket* clientSocket, const QJsonObject& response);
  void sendReponse(const QJsonObject& response);

private Q_SLOTS:
  void getResponse(const QJsonObject& response);

};

#endif // REMOTECOMMANDS_H
