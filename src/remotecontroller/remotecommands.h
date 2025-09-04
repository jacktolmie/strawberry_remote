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

  // List of basic commands. Forward to basicCommands if found.
  QList<QString> basicCommandsMap{
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

public:
  explicit RemoteCommands(Application *app, QObject *parent);
  ~RemoteCommands() = default;

  void processCommand(QTcpSocket *clientSocket, const QString& command, const QStringList& args);

public Q_SLOTS:
  void processLine(QTcpSocket* clientSocket, const QString& line);

Q_SIGNALS:
  void forwardToPlayer(const QString& command, const QStringList& args);
  void sendReponse(QTcpSocket* clientSocket, const QJsonObject& response);

private Q_SLOTS:
  void getResponse(QTcpSocket* clientSocket, const QJsonObject& playlists);

};

#endif // REMOTECOMMANDS_H
