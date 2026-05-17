#include <QJsonDocument>
#include <QHostInfo>
#include <QRandomGenerator>
#include "remotecontroller.h"
#include "core/logging.h"
#include <QCoreApplication>

#include <QString>
#include "core/player.h"
#include "remotecontroller/remotejsoncreator.h"

using namespace Qt::Literals::StringLiterals;

RemoteController::RemoteController(const Application* app, const Ui_MainWindow *mainUi , QObject *parent)
    : QObject{parent},
      app_{app},
      mainUi_{mainUi}
{
  commands = new RemoteCommands(const_cast<Application*>(app_), this);
  connect(this, &RemoteController::commandReceived, commands, &RemoteCommands::processLine);
  connect(commands, &RemoteCommands::sendReponse, this, &RemoteController::broadcastToDevices);
  // connect(commands, &RemoteCommands::sendReponse, this, &RemoteController::onSendResponse);


  server = new QTcpServer(this);

  // Create a timer to check network connection.
  timer = new QTimer(this);
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
      ClientInfo* client = new ClientInfo();
      client->socket = socket;
      clients_.insert(socket, client);

      // If no password required, do not check.
      if (!app_->remote_settings()->values.authRequired) {
        client->state = ClientState::Authenticated;
        onSendResponse(socket, RemoteJsonCreator::createResponse({ {u"auth"_s, u"AUTH_SUCCESS\n"_s }}));
        // socket->write("AUTH_SUCCESS\n");
      }
      else {
        QByteArray nonce(32, Qt::Uninitialized);
        for(int i{0}; i < nonce.size(); ++i){
          nonce[i] = static_cast<char>(QRandomGenerator::global()->generate64() & 0xFF);
        }

        client->nonce = nonce;
        // client->nonce = QByteArray::number(QRandomGenerator::global()->generate64());
        client->state = ClientState::ChallengeSent;

        onSendResponse(socket, { {u"auth"_s, u"CHALLENGE"_s}, {u"nonce"_s, QString::fromLatin1(nonce.toBase64())} });
        // socket->write("CHALLENGE " + client->nonce.toHex() + "\n");
        qDebug() << "Sent challenge (nonce) to client: " << client->nonce.toHex();
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

  ClientInfo* client = clients_.value(socket);

  switch (client->state) {

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

      if (!obj.contains(u"proof"_s)) {
        qDebug() << "No proof field in response. Kicking";
        socket->close();
        return;
      }

      QByteArray receivedProof = QByteArray::fromHex(obj[u"proof"_s].toString().toUtf8());
      QByteArray combined = client->nonce + app_->remote_settings()->values.password.toUtf8();
      QByteArray expectedProof = QCryptographicHash::hash(combined, QCryptographicHash::Sha256);

      if (receivedProof == expectedProof) {
        qDebug() << "Proof match for " << socket->peerAddress().toString();
        client->state = ClientState::Authenticated;
        client->nonce.clear();
        onSendResponse(socket, RemoteJsonCreator::createResponse({ {u"auth"_s, u"AUTH_SUCCESS"_s} }));
      } else {
        qDebug() << "Bad proof from " << socket->peerAddress().toString() << ". Kicking.";
        onSendResponse(socket, RemoteJsonCreator::createResponse({ {u"auth"_s, u"AUTH_FAILED"_s} }));
        socket->close();
      }
    break;
}


      // QString line = QString::fromUtf8(socket->readLine().trimmed());
      // if (!line.startsWith(u"PROOF "_s)) {
      //   qDebug() << "Bad protocol from client. Kicking.";
      //   socket->close();
      //   return;
      // }

      // QByteArray receivedProof = QByteArray::fromHex(line.mid(6).toUtf8());
      // QByteArray combined = client->nonce + app_->remote_settings()->values.password.toUtf8();
      // QByteArray expectedProof = QCryptographicHash::hash(combined, QCryptographicHash::Sha256);

      // if (receivedProof == expectedProof) {
      //   qDebug() << "Proof match for " << socket->peerAddress().toString() << ". Client is now authenticated";

      //   client->state = ClientState::Authenticated;
      //   client->nonce.clear();

      //   onSendResponse(socket, RemoteJsonCreator::createResponse({ {u"auth"_s, u"AUTH_SUCCESS\n"_s }}));
      //   // socket->write("AUTH_SUCCESS\n");
      // }
      // else {
      //   qDebug() << "Bad proof from " << socket->peerAddress().toString() << ". Kicking";
      //   onSendResponse(socket, RemoteJsonCreator::createResponse({ {u"auth"_s, u"AUTH_FAILED\n"_s }}));
      //   // socket->write("AUTH_FAILED\n");
      //   socket->close();
      //   return;
      // }
      // break;
    // }
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
        Q_EMIT RemoteController::commandReceived(socket, QString::fromUtf8(jsonData));
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
    ClientInfo* client = clients_.value(socket);
    clients_.remove(socket);
    delete client;
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
  if (clients_.contains(clientSocket)) {
      qDebug() << "Client state:" << static_cast<int>(clients_.value(clientSocket)->state);
  }
      if (clientSocket &&
        clientSocket->state() == QAbstractSocket::ConnectedState &&
        clients_.contains(clientSocket)
      )
      {
        QByteArray json = QJsonDocument(response).toJson(QJsonDocument::Compact);
        QDataStream socketStream(clientSocket);
        socketStream.setVersion(QDataStream::Qt_6_8);
        socketStream << json;
        // QByteArray json = QJsonDocument(response).toJson(QJsonDocument::Compact);

        // json.append('\n');
        // qInfo() << "Json sent: " << json;

        // clientSocket->write(json);

        // clientSocket->write(QByteArray(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n"));
        // QDataStream socketStream(clientSocket);
        // socketStream.setVersion(QDataStream::Qt_6_8);
        // socketStream << QJsonDocument(response).toJson(QJsonDocument::Compact);
      }
  }

void RemoteController::broadcastToDevices(const QJsonObject& message)
{
  // Send each authenticated client the broadcast from the server.
  for(auto clientSocket: std::as_const(clients_)){
    if(clientSocket->state == ClientState::Authenticated){
      onSendResponse(clientSocket->socket, message);
    }
  }
}

