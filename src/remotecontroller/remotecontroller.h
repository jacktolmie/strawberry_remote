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
#include "ui_mainwindow.h"
#include "remotecontroller/remotesettings.h"
// #include "remotecontroller/remotecommands.h"
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
  explicit RemoteController(const Application* app, const Ui_MainWindow* mainUi , QObject *parent = nullptr);
  // explicit RemoteController(const SharedPtr<RemoteSettings> data, QObject *parent = nullptr);
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

  // SharedPtr<RemoteSettings> data_;
  const Application   *app_;
  const Ui_MainWindow *mainUi_;

Q_SIGNALS:
  void ExitFinished();
  void commandReceived(QTcpSocket* clientSocket, const QString& line);

public Q_SLOTS:
  void activeNetworkConnection();
  void onSendResponse(QTcpSocket* clientSocket, const QJsonObject& response);
  void settingsChanged(const Values& data);

private Q_SLOTS:
  void ExitReceived();

  // Network port objects.
  void onNewConnection();
  void onReadyRead();
  void onDisconnect();

};

#endif // REMOTECONTROLLER_H
