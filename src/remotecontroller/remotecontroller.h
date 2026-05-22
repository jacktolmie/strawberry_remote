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
#include "remotecontroller/remoteguivalues.h"
#include "remotecontroller/remotesettings.h"
#include "remotecontroller/remotecommands.h"
#include "playlist/playlist.h"

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
  ~RemoteController() = default;

  void Exit();
  void serverCheck();
  void setTimer();

private:
  QTcpServer        *server;
  QHostAddress      localIp4;
  QHostAddress      localIp6;


  const Application   *app_;
  const Ui_MainWindow *mainUi_;
  RemoteGuiValues   guiValues;



  QTimer        *timer;

  RemoteCommands *commands;

  // List of connected sockets, and if authenticated
  QHash<QTcpSocket*, ClientInfo*> clients_;

Q_SIGNALS:
  void ExitFinished();
  void commandReceived(const QString& line);

public Q_SLOTS:
  void activeNetworkConnection();
  void onSendResponse(QTcpSocket* clientSocket, const QJsonObject& response);
  void settingsChanged(const Values& data);

private Q_SLOTS:
  void ExitReceived();
  void broadcastToDevices(const QJsonObject& message);

  // Network port objects.
  void onNewConnection();
  void onReadyRead();
  void onDisconnect();

  void testJson(QTcpSocket*);

};

#endif // REMOTECONTROLLER_H
