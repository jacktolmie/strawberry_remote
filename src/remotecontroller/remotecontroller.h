#ifndef REMOTECONTROLLER_H
#define REMOTECONTROLLER_H

#include <QDebug>
#include <QObject>
#include <QNetworkInterface>
#include <QCryptographicHash>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QHash>
#include <QSettings>

#include "core/application.h"
#include "playlist/playlist.h"
#include "remoteguivalues.h"
#include "remotesettings.h"
#include "remotecommands.h"

enum class ClientState {
  Authenticated,
  ChallengeSent,
  Unauthenticated
};

struct ClientInfo {
  QTcpSocket *socket = nullptr;
  ClientState state = ClientState::Unauthenticated;
  QByteArray  nonce;
};

class RemoteController : public QObject
{
  Q_OBJECT

public:
  explicit RemoteController(const Application* app, QObject *parent = nullptr);
  ~RemoteController() = default;

  void Exit();
  void serverCheck();
  void setTimer();

private:
  QTcpServer            *server;
  QHostAddress          localIp4;
  QHostAddress          localIp6;

  const Application     *app_;
  const RemoteCommands  *commands;  
  const RemoteGuiValues *guiValues;

  QTimer                *timer;

  void onSendResponse(QTcpSocket* clientSocket, const QJsonObject& response);

  // List of connected sockets, and if authenticated
  QHash<QTcpSocket*, ClientInfo> clients_;

Q_SIGNALS:
  void ExitFinished();
  void commandReceived(const QString& line);

public Q_SLOTS:
  void activeNetworkConnection();
  void settingsChanged(const Values& data);

private Q_SLOTS:
  void ExitReceived();
  void broadcastToDevices(const QJsonObject& message);

  // Network port objects.
  void onNewConnection();
  void onReadyRead();
  void onDisconnect();
};

#endif // REMOTECONTROLLER_H
