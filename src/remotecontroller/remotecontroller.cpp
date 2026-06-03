#include <QJsonDocument>
#include <QHostInfo>
#include <QRandomGenerator>
#include "remotecontroller.h"
#include "core/logging.h"
#include <QCoreApplication>

#include <QString>
#include "core/player.h"
#include "remotecontroller/remotejsoncreator.h"
#include "remotecontroller/remotetypes.h"

using namespace Qt::Literals::StringLiterals;
using namespace RemoteTypes;

RemoteController::RemoteController(const Application* app, QObject *parent)
    : QObject{parent},
      app_{app},
      // mainUi_{mainUi},
      guiValues{RemoteGuiValues(app_, this)}
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
          onSendResponse(socket, RemoteJsonCreator::createResponse({
            field(MessageType::AUTH, toString(MessageType::AUTH)),
            field(Auth::AUTH, toString(Auth::AUTH_SUCCESS))
          }));
      }
      else {
        QByteArray nonce(32, Qt::Uninitialized);
        for(int i{0}; i < nonce.size(); ++i){
          nonce[i] = static_cast<char>(QRandomGenerator::global()->generate64() & 0xFF);
        }

        client->nonce = nonce;
        client->state = ClientState::ChallengeSent;

        onSendResponse(socket, RemoteJsonCreator::createResponse({
          field(MessageType::AUTH, toString(MessageType::AUTH)),
          field(Auth::AUTH, toString(Auth::CHALLENGE)),
          field(Arguments::NONCE, QString::fromLatin1(nonce.toBase64()))
        }));
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
        onSendResponse(socket, RemoteJsonCreator::createResponse({
          field(MessageType::AUTH, toString(MessageType::AUTH)),
          field(Auth::AUTH, toString(Auth::AUTH_SUCCESS))
      }));
      } else {
        qDebug() << "Bad proof from " << socket->peerAddress().toString() << ". Kicking.";
        onSendResponse(socket, RemoteJsonCreator::createResponse({
          field(MessageType::AUTH, toString(MessageType::AUTH)),
          field(Auth::AUTH, toString(Auth::AUTH_FAILED))
      }));
        socket->close();
      }
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
        // onSendResponse(socket, guiValues.triggerUpdate());

      }

      break;
    }
    default: {
      socket->close();
      break;
    }
  }
    //testJson(socket); // delete after running JSON send tests.
    onSendResponse(socket, guiValues.triggerUpdate());
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
  // Send each authenticated client the broadcast from the server.
  for(auto clientSocket: std::as_const(clients_)){
    if(clientSocket->state == ClientState::Authenticated){
      onSendResponse(clientSocket->socket, message);
    }
  }
}

void RemoteController::testJson(QTcpSocket* socket){

  QVector<QJsonObject> allJsonResponses;
/*
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::EVENT, toString(MessageType::EVENT)),
    field(Event::EVENT, toString(Event::GUI_UPDATES)),
    field(Arguments::VOLUME, static_cast<qint32>(app_->player()->GetVolume())),
    field(Arguments::CURRENT_TIME, 1000),
    field(Arguments::PLAYING, (app_->player()->GetState() == EngineBase::State::Playing)? true : false)
    // field(Arguments::PLAYLISTS, playlist.sendAllPlaylists())
  }));

  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::SONG_INFO)),
    field(Arguments::ID, 1),
    field(Arguments::TRACK_ID, 1)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::EVENT, toString(MessageType::EVENT)),
    field(Event::EVENT, toString(Event::SONG_CHANGED)),
    field(Arguments::TRACK_ID, 1)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::EVENT, toString(MessageType::EVENT)),
    field(Event::EVENT, toString(Event::ACTIVE_PLAYLIST)),
    field(Arguments::ID, 1),
    field(Arguments::ROW, 1)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::EVENT, toString(MessageType::EVENT)),
    field(Event::EVENT, toString(Event::VOLUME_CHANGED)),
    field(Arguments::VOLUME, static_cast<int>(1))
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::RUNNING_COMMAND)),
    field(Arguments::COMMAND, u"command"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::ERROR, toString(MessageType::ERROR)),
    field(Error::ERROR, toString(Error::COMMAND_NOT_FOUND)),
    field(Arguments::COMMAND, u"command"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::AUTH, toString(MessageType::AUTH)),
    field(Auth::AUTH, toString(Auth::AUTH_SUCCESS))
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::AUTH, toString(MessageType::AUTH)),
    field(Auth::AUTH, toString(Auth::AUTH_FAILED))
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::ERROR, toString(MessageType::ERROR)),
    field(Error::ERROR, toString(Error::NOT_ENOUGH_ARGUMENTS_PASSED_NEEDS)),
    field(Arguments::REQUIRED, 2)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::RENAME_PLAYLIST)),
    field(Arguments::NAME, u"songName"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::ERROR, toString(MessageType::ERROR)),
    field(Error::ERROR, toString(Error::PLAYLIST_NOT_FOUND)),
    field(Arguments::NAME, u"songName"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::SHUFFLED_ALL_PLAYLISTS))
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::ERROR, toString(MessageType::ERROR)),
    field(Error::ERROR, toString(Error::NOT_ENOUGH_ARGUMENTS_PASSED_NEEDS)),
    field(Arguments::REQUIRED, 1)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::ERROR, toString(MessageType::ERROR)),
    field(Error::ERROR, toString(Error::PLAYLIST_NOT_FOUND)),
    field(Arguments::NAME, u"name"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::ERROR, toString(MessageType::ERROR)),
    field(Error::ERROR, toString(Error::NOT_ENOUGH_ARGUMENTS_PASSED_NEEDS)),
    field(Arguments::REQUIRED, 2)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::IS_PLAYLIST_A_FAVOURITE)),
    field(Arguments::IS_FAVOURITE, true)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::ERROR, toString(MessageType::ERROR)),
    field(Error::ERROR, toString(Error::PLAYLIST_NOT_FOUND))
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::SET_CURRENT_PLAYLIST_TO)),
    field(Arguments::NAME, u"name"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::ERROR, toString(MessageType::ERROR)),
    field(Error::ERROR, toString(Error::WRONG_ARGUMENT_SENT)),
    field(Arguments::ARGUMENT, u"argument"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::PLAYLIST_CLOSED))
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::ERROR, toString(MessageType::ERROR)),
    field(Error::ERROR, toString(Error::PLAYLIST_NOT_CLOSED))
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::ERROR, toString(MessageType::ERROR)),
    field(Error::ERROR, toString(Error::WRONG_ARGUMENT_SENT)),
    field(Arguments::ARGUMENT, u"argument"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::DELETED_PLAYLIST_WITH_ID)),
    field(Arguments::ID, 1)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::CLOSED_PLAYLIST_WITH_ID)),
    field(Arguments::ID, 1)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::EVENT, toString(MessageType::EVENT)),
    field(Event::EVENT, toString(Event::FAVOURITE_PLAYLIST)),
    field(Arguments::ID, 1),
    field(Arguments::FAVOURITE, true)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::EVENT, toString(MessageType::EVENT)),
    field(Event::EVENT, toString(Event::RENAME_PLAYLIST)),
    field(Arguments::ID, 1),
    field(Arguments::NAME, u"name"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::EVENT, toString(MessageType::EVENT)),
    field(Event::EVENT, toString(Event::ACTIVE_PLAYLIST)),
    field(Arguments::ID, 1)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::SENT_ACTIVE_PLAYLIST)),
    field(Arguments::ID, 1)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::CLEARED_PLAYLIST)),
    field(Arguments::NAME, u"name"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::REMOVED_SONG_FROM_PLAYLIST)),
    field(Arguments::NAME, u"songName"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::REMOVED_DUPLICATES_FROM_PLAYLIST))
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::SHUFFLED_PLAYLIST))
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::EVENT, toString(MessageType::EVENT)),
    field(Event::EVENT, toString(Event::GUI_UPDATES)),
    field(Arguments::VOLUME, 1),
    field(Arguments::TIME, 1),
    field(Arguments::NONCE, (app_->player()->GetState() == EngineBase::State::Playing) ? true : false)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::ERROR, toString(MessageType::ERROR)),
    field(Error::ERROR, toString(Error::NOT_ENOUGH_ARGUMENTS_PASSED_NEEDS)),
    field(Arguments::REQUIRED, 2)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::RENAME_PLAYLIST)),
    field(Arguments::NAME, u"name"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::ERROR, toString(MessageType::ERROR)),
    field(Error::ERROR, toString(Error::PLAYLIST_NOT_FOUND)),
    field(Arguments::NAME, u"name"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::ERROR, toString(MessageType::ERROR)),
    field(Error::ERROR, toString(Error::PLAYLIST_NOT_FOUND)),
    field(Arguments::NAME, u"name"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::SET_CURRENT_PLAYLIST_TO)),
    field(Arguments::NAME, u"name"_s)
  }));
  allJsonResponses.push_back(RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::REMOVED_SONG_FROM_PLAYLIST)),
    field(Arguments::NAME, u"songName"_s)
  }));
*/
  for(const auto& ele: allJsonResponses) onSendResponse(socket, ele);

}