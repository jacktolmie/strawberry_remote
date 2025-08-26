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

#include "remotecontroller/remotesettings.h"
#include "includes/shared_ptr.h"

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
  explicit RemoteController(const SharedPtr<RemoteSettings> data, QObject *parent = nullptr);
  ~RemoteController() = default;

  void Exit();
  void serverCheck();
  void setTimer();

private:
  QTcpServer    *server;
  QHostAddress  localIp4;
  QHostAddress  localIp6;

  QTimer        *timer;

  // List of connected sockets, and if authenticated
  QHash<QTcpSocket*, ClientInfo*> clients_;

  SharedPtr<RemoteSettings> data_;
  // Values          values_;
Q_SIGNALS:
  void ExitFinished();

public Q_SLOTS:
  void settingsChanged(const Values& data);
  void activeNetworkConnection();

private Q_SLOTS:
  void ExitReceived();

  // Network port objects.
  void onNewConnection();
  void onReadyRead();
  void onDisconnect();

};

#endif // REMOTECONTROLLER_H
