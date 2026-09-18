#include <QJsonObject>
#include <QString>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>

#include "remotecommands.h"
#include "remotejsoncreator.h"
#include "remotetypes.h"

using namespace Qt::Literals::StringLiterals;
using namespace RemoteTypes;
using CommandHandler = std::function<QJsonObject(const QJsonObject&)>;

RemoteCommands::RemoteCommands(const Application* app, QObject* parent = nullptr):
    QObject{parent},
    app_(app),
    basicCommands_(new RemoteBasicCommands(app_)),
    remoteRadio_(new RemoteRadio(app_)),
    remotePlaylist_(new RemotePlaylist(app_, this)),
    basicCmdMap_{basicCommands_->sendCommandMap()},
    playlistCmdMap_{remotePlaylist_->sendCommandMap()},
    radioCmdMap_{remoteRadio_->sendCommandMap()}
{
    QObject::connect(remotePlaylist_, &RemotePlaylist::sendResponse, this, &RemoteCommands::sendResponse);
    QObject::connect(basicCommands_, &RemoteBasicCommands::sendResponse, this, &RemoteCommands::sendResponse);
    QObject::connect(remoteRadio_, &RemoteRadio::sendResponse, this, &RemoteCommands::sendResponse);

    commandMaps.append(&basicCmdMap_);
    commandMaps.append(&radioCmdMap_);
    commandMaps.append(&playlistCmdMap_);
}

void RemoteCommands::processCommand(const QString& command, const QJsonObject& args)
{
    if (command.isEmpty() || args.isEmpty()) return;

    for (const auto* map : std::as_const(commandMaps)) {
        auto it = map->find(command);
        if (it != map->end()) {
            it.value()(args);
            return;
        }
    }

    // If sent command does not match anything, send message back to device.
    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::ERROR, toString(MessageType::ERROR)),
        field(Error::ERROR, toString(Error::COMMAND_NOT_FOUND)),
        field(Arguments::COMMAND, command)
    }));
}

void RemoteCommands::processLine(const QString& line)
{
    if (line.isEmpty()) return;

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
    RemoteCommands::processCommand(command, obj);
}