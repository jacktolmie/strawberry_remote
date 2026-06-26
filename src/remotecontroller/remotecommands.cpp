#include <QJsonObject>
#include <QString>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>

#include "remotecommands.h"
#include "remotecontroller/remotejsoncreator.h"
#include "remotecontroller/remotetypes.h"

using namespace Qt::Literals::StringLiterals;
using namespace RemoteTypes;

RemoteCommands::RemoteCommands(const Application* app, QObject* parent = nullptr):
  QObject{parent},
  app_(app),
  remotePlaylist(new RemotePlaylist(app, this)),
  basicCommands(new RemoteBasicCommands(app)),
  basicCmdMap{basicCommands->sendCommandMap()},
  playlistCmdMap{remotePlaylist->sendCommandMap()}
{
  QObject::connect(remotePlaylist, &RemotePlaylist::sendResponse, this, &RemoteCommands::sendResponse);
  QObject::connect(basicCommands, &RemoteBasicCommands::sendResponse, this, &RemoteCommands::sendResponse);
}

void RemoteCommands::processCommand(const QString& command, const QStringList &args)
{
    // Check if sent command is in basicCommandMap.
    if(basicCmdMap.contains(command)){
        basicCmdMap[command](args);
        return;
    }
    else if (playlistCmdMap.contains(command)){
        playlistCmdMap[command](args);
        return;
    }
    else{
    // If sent command does not match anything, send message back to device.
        Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
          field(MessageType::ERROR, toString(MessageType::ERROR)),
          field(Error::ERROR, toString(Error::COMMAND_NOT_FOUND)),
          field(Arguments::COMMAND, command)
          }));
      }
}

void RemoteCommands::processLine(const QString& line)
{
    qInfo() << "Command processline: " << line;
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

    QString command{u"command"_s};
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
    //    This handles cases like { "command": "rename", "args": ["oldName", "newName"]
    else if (obj.contains(arg) && obj[arg].isArray()) {
        QJsonArray argArray = obj[arg].toArray();
        for (const QJsonValue& val : std::as_const(argArray)) {
            args.append(val.toString());
        }
    }
    else {
        const QStringList keys = obj.keys();
        QMap<int, QString> orderedArgs; // Use QMap to store args in order by their integer key
        QStringList namedArgs;

        // Create list of words to be skipped from incoming messages.
        const QStringList reservedKeys = { u"command"_s };

        for (const QString& key : keys) {
            if (reservedKeys.contains(key)) continue; // skip words like "command"
            bool isNumber;
            int index = key.toInt(&isNumber); // Try to convert key to an integer
            // Make sure it's a non-negative number and the value is a string
            if (isNumber && index >= 0 && obj[key].isString()) {
                orderedArgs.insert(index, obj[key].toString());
            }
            else{
              namedArgs.append(obj[key].toVariant().toString());
            }
        }
        // Add all collected and ordered arguments to the args list
        for (const QString& argVal : orderedArgs) {
            args.append(argVal);
        }
        args.append(namedArgs);
    }

    // Process command with args after breaking down the JSON file.
    RemoteCommands::processCommand(command, args);
}
