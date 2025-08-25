#include "remotecontroller.h"


RemoteController::RemoteController(const SharedPtr<RemoteSettings> data, QObject *parent)
    : QObject{parent},
      data_(data)
{
  server = new QTcpServer(this);

  // Create a timer to check network connection.
  timer = new QTimer(this);
  timer->setInterval(5000);
  connect(timer, &QTimer::timeout, this, &RemoteController::activeNetworkConnection);
  RemoteController::setTimer();
}

void RemoteController::setTimer()
{
  qDebug() << "setTimer called. Remote enabled? " << data_->values.remoteEnabled;

  data_->values.remoteEnabled ? timer->start() : timer->stop();
  qDebug() << "Remote Is timer active? " << timer->isActive();
}

void RemoteController::serverCheck()
{
  if (data_->values.remoteEnabled){
    // Check for active connection.
    RemoteController::activeNetworkConnection();

    // Shut down and restart the server.
    if(server->isListening()) server->close();
    server->listen(QHostAddress::Any, data_->values.portNumber);

    if (!server->isListening()) {
      qDebug() << "Failed to start server on new port.";
      // Handle error...
    }
    else {
        qDebug() << "Remote Server running? " << server->isListening() << " on port: " << data_->values.portNumber;
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
        qDebug() << "Remote Found active, usable IPv4 address:" << ip.toString() << "on interface" << interface.name();
        data_->values.activeNetwork = true;
        return;
      }
    }
  }
  qDebug() << "Remote activeNetworkConnection called. No active networks found";
  data_->values.activeNetwork = false;
}

// void RemoteController::ExitFinished(){}

void RemoteController::Exit(){
  qDebug() << "RemoteController ExitFinished called";
  Q_EMIT ExitFinished();
}

void RemoteController::ExitReceived(){
  qDebug() << "RemoteController ExitReceived called";
}

void RemoteController::onNewConnection()
{
  qDebug() << "Remote onNewConnection called";
  // Check for any incoming connections.
  while (server->hasPendingConnections()) {
    QTcpSocket *socket = server->nextPendingConnection();

    if (socket) {
      qDebug() << "New client connecting from" << socket->peerAddress().toString();
      // Create a new client for clients_ list.
      ClientInfo* client = new ClientInfo();
      client->socket = socket;
      client->state = ClientState::Unauthenticated;
      clients_.insert(socket, client);

      connect(socket, &QTcpSocket::readyRead, this, &RemoteController::onReadyRead);
      connect(socket, &QTcpSocket::disconnected, this, &RemoteController::onDisconnect);
    }
  }
}

void RemoteController::onReadyRead(){

  QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

  // If socket is nullprt, or the client is not in the list of clients_, return
  if (!socket || !clients_.contains(socket)) return;

  qDebug() << "Remote onReadyRead called";

  ClientInfo* client = clients_.value(socket);

  switch (client->state) {
    case ClientState::Unauthenticated: {
      // Wait for full line of data to arrive before processing.
      if (!socket->canReadLine()) return;

      QByteArray receivedPassword = socket->readLine().trimmed();

      if (data_->values.hashedPassword == receivedPassword) {
        qDebug() << "Password matched for" << socket->peerAddress().toString() << ". Client is now authenticated.";

        client->state = ClientState::Authenticated;

        // Send client acknowledgemnet it is authenticated.
        socket->write("AUTH_SUCCESS\n");
      }
      else {
        qDebug() << "Bad password from" << socket->peerAddress().toString() << ". Kicking them out.";
        socket->write("AUTH_FAILURE\n");
        socket->close();
      }

      break;
    }
    case ClientState::Authenticated: {
      while (socket->canReadLine()) {
        QString line = QString::fromUtf8(socket->readLine().trimmed());
        qDebug() << "Authenticated client" << socket->peerAddress().toString() << "sent command:" << line;

        // Now you can parse this line for commands like "play", "pause", "volume", etc.
        // QStringList parts = line.split(' ');
        // QString command = parts[0];
        // if (command == "volume") { ... }
      }

      break;
    }

  }

  while (socket->canReadLine()){
    QByteArray line = socket->readLine().trimmed();

    qDebug() << "Remote line: " <<line;




  }
}

void RemoteController::onDisconnect()
{
  QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
  // If socket is nullptr, return.
  if (!socket) return;

  qDebug() << "Client disconnected, cleaning up session for" << socket->peerAddress().toString();
  if (clients_.contains(socket)){
    ClientInfo* client = clients_.value(socket);
    clients_.remove(socket);
    delete client;
  }

  socket->deleteLater();
}

void RemoteController::settingsChanged(const Values& data)
{
  qDebug() << "Remote controller settings changed called " <<"Port: "<<data_->values.portNumber<<" Remote Enabled: "<< data_->values.remoteEnabled;

  RemoteController::serverCheck();
  // RemoteController::setTimer();

}
