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

  // If the command is a basic command, run it in RemoteBasicCommands.
  auto basicCommand{basicCommands.sendCommand(command, args)};

  // Check if basicCommand ran the command or it was not found.
  if (basicCommand.contains(QStringLiteral("response"))){
    QString response{basicCommand.value(QStringLiteral("response")).toString()};

    // If the command was in the basic commands, it will return running. Send response back.
    if (response.contains(QStringLiteral("Running"))) RemoteCommands::getResponse(clientSocket, basicCommand);

    // If the command was not in basic commands, continue looking for the command.
    else if (response.contains(QStringLiteral("not")) ){
      // If the command is not a basic command, check other command functions.
      if (command.contains(QStringLiteral("playlist"))){
        playlist.processCommand(clientSocket, command, args);
      }
    }
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
