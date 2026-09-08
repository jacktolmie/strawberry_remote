#include <QJsonDocument>
#include <QHostInfo>
#include <QRandomGenerator>
#include <QCoreApplication>
#include <QString>

#include "core/logging.h"
#include "core/player.h"
#include "remotecontroller.h"
#include "remotejsoncreator.h"
#include "remotetypes.h"

using namespace Qt::Literals::StringLiterals;
using namespace RemoteTypes;

RemoteController::RemoteController(const Application* app, QObject *parent)
    : QObject{parent},
      server{new QTcpServer(this)},
      app_{app},
      commands_{new RemoteCommands(app, this)},
      guiValues_{new RemoteGuiValues(commands_->getRemotePlaylist(), app_, this)},
      timer{new QTimer(this)}
{
  connect(this, &RemoteController::commandReceived, commands_, &RemoteCommands::processLine);
  connect(commands_, &RemoteCommands::sendResponse, this, &RemoteController::broadcastToDevices);
  connect(&*app_->player(), &Player::sendToRemote, this, &RemoteController::broadcastToDevices);
  connect(guiValues_, &RemoteGuiValues::sendCurrentStatus, this, &RemoteController::broadcastToDevices);

  // Create a timer to check network connection.
  timer->setInterval(30000);
  connect(timer, &QTimer::timeout, this, &RemoteController::activeNetworkConnection);
  connect(server, &QTcpServer::newConnection, this, &RemoteController::onNewConnection);

  RemoteController::setTimer();
  RemoteController::serverCheck();
}

void RemoteController::setTimer()
{
  app_->remote_settings()->values.remoteEnabled ? timer->start() : timer->stop();
}

void RemoteController::serverCheck()
{
  if (app_->remote_settings()->values.remoteEnabled){
    // Check for active connection.
    RemoteController::activeNetworkConnection();

    // Shut down and restart the server.
    if(server->isListening()) server->close();
    server->listen(QHostAddress::Any, app_->remote_settings()->values.portNumber);

    if (!server->isListening()) {
      qDebug() << "Failed to start server on new port.";
      // Handle error...
    }
    else {
        qDebug() << "Remote Server running? " << server->isListening() << " on port: " << app_->remote_settings()->values.portNumber;
    }
  }
  else {
    qDebug() << "Remote not enabled";
    server->close(); // Shut down server.
    qDebug() << "Remote Shutting down server... Server still running? " << server->isListening();
  }
  RemoteController::setTimer();
}

void RemoteController::activeNetworkConnection()
{
  // Check for active network. If found, disable no network warning, otherwise show it.
  const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
  for (const QNetworkInterface &interface : interfaces) {
    // Get all IP address entries for this interface
    const QList<QNetworkAddressEntry> entries = interface.addressEntries();
    for (const QNetworkAddressEntry &entry : entries) {
      QHostAddress ip = entry.ip();

      if (ip.protocol() == QAbstractSocket::IPv4Protocol && !ip.isLoopback()) {
        qLog(Info) << "Remote Found active, usable IPv4 address:" << ip.toString() << "on interface" << interface.name();
        app_->remote_settings()->values.activeNetwork = true;
        return;
      }
    }
  }
  qLog(Warning) << "Remote activeNetworkConnection called. No active networks found";
  app_->remote_settings()->values.activeNetwork = false;
}

void RemoteController::Exit(){
  Q_EMIT ExitFinished();
}

void RemoteController::ExitReceived(){}

void RemoteController::onNewConnection()
{
  // Check for any incoming connections.
  while (server->hasPendingConnections()) {
    QTcpSocket *socket = server->nextPendingConnection();

    if (socket) {
      qDebug() << "Remote New client connecting from" << socket->peerAddress().toString();
      // Create a new client for clients_ list.
      clients_.insert(socket, ClientInfo());
      ClientInfo& client = clients_[socket];
      client.socket = socket;

      // If no password required, do not check.
      if (!app_->remote_settings()->values.authRequired) {
        client.state = ClientState::Authenticated;
          onSendResponse(socket, RemoteJsonCreator::createResponse({
            field(MessageType::AUTH, toString(MessageType::AUTH)),
            field(Auth::AUTH, toString(Auth::AUTH_SUCCESS))
          }));
        onSendResponse(socket, guiValues_->triggerUpdate());
      }
      else {
        QByteArray nonce(32, Qt::Uninitialized);
        for(int i{0}; i < nonce.size(); ++i){
          nonce[i] = static_cast<char>(QRandomGenerator::global()->generate64() & 0xFF);
        }

        client.nonce = nonce;
        client.state = ClientState::ChallengeSent;

        onSendResponse(socket, RemoteJsonCreator::createResponse({
          field(MessageType::AUTH, toString(MessageType::AUTH)),
          field(Auth::AUTH, toString(Auth::CHALLENGE)),
          field(Arguments::NONCE, QString::fromLatin1(nonce.toBase64()))
        }));
        qDebug() << "Sent challenge (nonce) to client: " << client.nonce.toHex();
      }

      connect(socket, &QTcpSocket::readyRead, this, &RemoteController::onReadyRead);
      connect(socket, &QTcpSocket::disconnected, this, &RemoteController::onDisconnect);
    }
  }
}

void RemoteController::onReadyRead()
{
  QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

  // If socket is nullprt, or the client is not in the list of clients_, return
  if (!socket || !clients_.contains(socket)) return;

  // ClientInfo* client = clients_.value(socket);
  ClientInfo client = clients_.value(socket);

  switch (client.state) {

    case ClientState::ChallengeSent: {
      QDataStream socketStream(socket);
      socketStream.setVersion(QDataStream::Qt_6_8);

      socketStream.startTransaction();
      QByteArray jsonData;
      socketStream >> jsonData;

      if (!socketStream.commitTransaction()) return;

      QJsonDocument doc = QJsonDocument::fromJson(jsonData);

      if (doc.isNull() || !doc.isObject()) {
        qDebug() << "Bad protocol from client. Kicking.";
        socket->close();
        return;
      }

      QJsonObject obj = doc.object();

      if (!obj.contains(toString(Arguments::PROOF))) {
        qDebug() << "No proof field in response. Kicking";
        socket->close();
        return;
      }

      QByteArray receivedProof = QByteArray::fromHex(obj[toString(Arguments::PROOF)].toString().toUtf8());
      QByteArray combined = client.nonce + app_->remote_settings()->values.password.toUtf8();
      QByteArray expectedProof = QCryptographicHash::hash(combined, QCryptographicHash::Sha256);

      if (receivedProof == expectedProof) {
        qDebug() << "Proof match for " << socket->peerAddress().toString();
        client.state = ClientState::Authenticated;
        client.nonce.clear();
        onSendResponse(socket, RemoteJsonCreator::createResponse({
          field(MessageType::AUTH, toString(MessageType::AUTH)),
          field(Auth::AUTH, toString(Auth::AUTH_SUCCESS))
      }));
        onSendResponse(socket, guiValues_->triggerUpdate());

      } else {
        qDebug() << "Bad proof from " << socket->peerAddress().toString() << ". Kicking.";
        onSendResponse(socket, RemoteJsonCreator::createResponse({
          field(MessageType::AUTH, toString(MessageType::AUTH)),
          field(Auth::AUTH, toString(Auth::AUTH_FAILED))
      }));
        socket->close();
      }

    clients_[socket] = client;

    break;
}
    case ClientState::Authenticated: {

      QDataStream socketStream(socket);
      socketStream.setVersion(QDataStream::Qt_6_8);


      while(true){
        socketStream.startTransaction();
        QByteArray jsonData;
        socketStream >> jsonData;

        if (!socketStream.commitTransaction()){
          break;
        }
        qDebug() << "Authenticated client" << socket->peerAddress().toString() << "sent command:" << QString::fromUtf8(jsonData);
        Q_EMIT RemoteController::commandReceived(QString::fromUtf8(jsonData));
      }

      break;
    }
    default: {
      socket->close();
      break;
    }
  }
}

void RemoteController::onDisconnect()
{
  QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
  // If socket is nullptr, return.
  if (!socket) return;

  qLog(Info) << "Client disconnected, cleaning up session for" << socket->peerAddress().toString();
  if (clients_.contains(socket)){
    ClientInfo client = clients_.value(socket);
    clients_.remove(socket);
    // delete client;
  }

  socket->deleteLater();
}

void RemoteController::settingsChanged(const Values& data)
{
  this->app_->remote_settings()->values = data;
  RemoteController::serverCheck();
}

void RemoteController::onSendResponse(QTcpSocket* clientSocket, const QJsonObject& response)
{
  if (clientSocket &&
    clientSocket->state() == QAbstractSocket::ConnectedState &&
    clients_.contains(clientSocket)
  )
  {
    QByteArray json = QJsonDocument(response).toJson(QJsonDocument::Compact);
    QDataStream socketStream(clientSocket);
    socketStream.setVersion(QDataStream::Qt_6_8);
    socketStream << json;
  }
}

void RemoteController::broadcastToDevices(const QJsonObject& message)
{
    // Delete if statement when done testing
    if(!message.contains(u"cover_image"_s))
    qInfo()<< "broadcasttodevices called with message: "<< message;

  // Send each authenticated client the broadcast from the server.
  for(auto &clientSocket: std::as_const(clients_)){
    if(clientSocket.state == ClientState::Authenticated){
      onSendResponse(clientSocket.socket, message);
    }
  }
}