#include <QJsonObject>
#include <QString>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>

#include "remotecommands.h"


RemoteCommands::RemoteCommands(Application* app, QObject* parent = nullptr):
  QObject{parent},
  app_(app),
  playlist(RemotePlaylist(app, this)),
  basicCommands(RemoteBasicCommands(app))
{
  connect(&playlist, &RemotePlaylist::sendResponse, this, &RemoteCommands::sendReponse);
}

void RemoteCommands::processCommand(QTcpSocket* clientSocket, const QString& command, const QStringList &args)
{
  qDebug() <<"RemoteCommand::processCommand called";

  // Check if sent command is in basicCommandMap.
  if(basicCommandsMap.contains(command)){
    RemoteCommands::getResponse(clientSocket, basicCommands.sendCommand(command, args));
    return;
  }
  if (command.contains(QStringLiteral("playlist"))){
    playlist.processCommand(clientSocket, command, args);
    return;
  }

  // If sent command does not match anything, send message back to device.
  RemoteCommands::getResponse(clientSocket,
    QJsonObject{{QStringLiteral("response"), QStringLiteral("The command %1 was not found").arg(command)}});
}

void RemoteCommands::processLine(QTcpSocket *clientSocket, const QString& line)
{
  qDebug() << "RemoteCommands::processLine called with: "<< line;

  QJsonParseError parseError;
  QJsonDocument doc{QJsonDocument::fromJson(line.toUtf8(), &parseError)};

  if (parseError.error != QJsonParseError::NoError) {
    qWarning() << "Failed to parse JSON command: "<< parseError.errorString();
    return;
  }

  if (!doc.isObject()) {
    qWarning() << "Received JSON is not an object.";
    return;
  }

  QJsonObject obj{doc.object()};

  QString command{QStringLiteral("command")};
  if (!obj.contains(command) || !obj[command].isString()) {
    qWarning() << "JSON command is missing a 'command' string field.";
    return;
  }

  command = obj[command].toString().toLower();

  QStringList args;
  QString value{QStringLiteral("value")};
  QString arg{QStringLiteral("args")};

    if (obj.contains(value)) {
        args.append(obj[value].toVariant().toString());
    }
    //    This handles cases like { "command": "rename", "args": ["oldName", "newName"] }
    if (obj.contains(arg) && obj[arg].isArray()) {
        QJsonArray argArray = obj[arg].toArray();
        for (const QJsonValue& val : std::as_const(argArray)) {
            args.append(val.toString());
        }
    }
  qDebug() << "Remote Final arguments for command '" << command << "':" << args;

  // Process command with args after breaking down the JSON file.
  RemoteCommands::processCommand(clientSocket, command, args);
}

void RemoteCommands::getResponse(QTcpSocket *clientSocket, const QJsonObject& response)
{

  Q_EMIT RemoteCommands::sendReponse(clientSocket, response);
}
// void RemoteCommands::getAllPlaylists(const QJsonObject& playlists){}
// void RemoteCommands::getCurrentPlaylist(const QJsonObject& playlist){}
