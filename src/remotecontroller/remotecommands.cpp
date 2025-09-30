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
  basicCommands(RemoteBasicCommands(app)),
  values{new RemoteGuiValues(app, this)},
  basicCmdMap{basicCommands.sendCommandMap()}
{
  connect(&playlist, &RemotePlaylist::sendResponse, this, &RemoteCommands::getResponse);
  connect(&basicCommands, &RemoteBasicCommands::sendResponse, this, &RemoteCommands::getResponse);
  // connect(values, &RemoteGuiValues::sendCurrentStatus, this, &RemoteCommands::getGuiUpdate);

}

void RemoteCommands::processCommand(QTcpSocket* clientSocket, const QString& command, const QStringList &args)
{
  // Check if sent command is in basicCommandMap.
  if(basicCmdMap.contains(command)){
    RemoteCommands::getResponse(basicCommands.checkCommand(command, args));
  }
  else if (command.contains(QStringLiteral("playlist"))){
    playlist.processCommand(command, args);
  }
  else{
  // If sent command does not match anything, send message back to device.
    RemoteCommands::getResponse(QJsonObject{{QStringLiteral("response"), QStringLiteral("The command %1 was not found").arg(command)}});
  }

  if(values) values->triggerUpdate(clientSocket);
}

void RemoteCommands::processLine(QTcpSocket *clientSocket, const QString& line)
{
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

void RemoteCommands::getResponse(const QJsonObject& response)
{
  Q_EMIT RemoteCommands::sendReponse(response);
}

void RemoteCommands::getGuiUpdate(QTcpSocket *client, QJsonObject updates){

}

