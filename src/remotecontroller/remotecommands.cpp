#include <QJsonObject>
#include <QString>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>

#include "remotecommands.h"
#include "remotecontroller/remotejsoncreator.h"
#include "playlist/playlistmanager.h"
#include "core/player.h"

using namespace Qt::Literals::StringLiterals;

RemoteCommands::RemoteCommands(Application* app, QObject* parent = nullptr):
  QObject{parent},
  app_(app),
  playlist(RemotePlaylist(app, this)),
  basicCommands(RemoteBasicCommands(app)),
  values{new RemoteGuiValues(app, this)},
  basicCmdMap{basicCommands.sendCommandMap()},
  playlistCmdMap{playlist.sendCommandMap()}
{
  QObject::connect(&playlist, &RemotePlaylist::sendResponse, this, &RemoteCommands::getResponse);
  QObject::connect(&basicCommands, &RemoteBasicCommands::sendResponse, this, &RemoteCommands::getResponse);
  QObject::connect(&*app_->player(), &Player::sendToRemote, this, &RemoteCommands::getResponse);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::sendPlaylistResponse, this, &RemoteCommands::getResponse);
  // connect(values, &RemoteGuiValues::sendCurrentStatus, this, &RemoteCommands::getGuiUpdate);
}

void RemoteCommands::processCommand(QTcpSocket* clientSocket, const QString& command, const QStringList &args)
{
  // Check if sent command is in basicCommandMap.
  if(basicCmdMap.contains(command)){
    basicCmdMap[command](args);
    RemoteCommands::getResponse(RemoteJsonCreator::createResponse({ {u"response"_s, u"running_command: "_s}, {u"command"_s, command} }));
  }
  else if (playlistCmdMap.contains(command)){
    RemoteCommands::getResponse(playlistCmdMap[command](args));
  }
  else{
  // If sent command does not match anything, send message back to device.
    RemoteCommands::getResponse(RemoteJsonCreator::createResponse({ {u"error"_s, u"the_command_was_not_found"_s}, {u"command"_s, command} }));
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

  QString command{u"event"_s};
  if (!obj.contains(command) || !obj[command].isString()) {
    qWarning() << "JSON command is missing a 'command' string field.";
    return;
  }

  command = obj[command].toString().toLower();

  QStringList args;
  QString value{u"value"_s};
  QString arg{u"args"_s};

    if (obj.contains(value)) {
        args.append(obj[value].toVariant().toString());
    }
    //    This handles cases like { "event": "rename", "args": ["oldName", "newName"] }
    else if (obj.contains(arg) && obj[arg].isArray()) {
        QJsonArray argArray = obj[arg].toArray();
        for (const QJsonValue& val : std::as_const(argArray)) {
            args.append(val.toString());
        }
    }
    else {
        const QStringList keys = obj.keys();
        QMap<int, QString> orderedArgs; // Use QMap to store args in order by their integer key
        for (const QString& key : keys) {
            bool isNumber;
            int index = key.toInt(&isNumber); // Try to convert key to an integer
            // Make sure it's a non-negative number and the value is a string
            if (isNumber && index >= 0 && obj[key].isString()) {
                orderedArgs.insert(index, obj[key].toString());
            }
        }
        // Add all collected and ordered arguments to the args list
        for (const QString& argVal : orderedArgs) {
            args.append(argVal);
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

