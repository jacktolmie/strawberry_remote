#include "remotecommands.h"


RemoteCommands::RemoteCommands(Application* app, QObject* parent = nullptr):
  QObject{parent},
  app_(app),
  playlist(RemotePlaylist(app, this)),
  basicCommands(RemoteBasicCommands(app))
{}

// QString RemoteCommands::processCommand(const QString& command, const QStringList &args)
void RemoteCommands::processCommand(const QString& command, const QStringList &args)
{
  qDebug() << "RemoteCommands::processCommand called with: "<< command;

  // If the command is a basic command, run it in RemoteBasicCommands.
  if (!basicCommands.sendCommand(command, args) ){
    // If the command is not a basic command, check other command functions.
    qDebug() << "Returned from RemoteBasicCommand send command call. Not basic command";
    if (command.contains(QStringLiteral("playlist"))){
      playlist.processCommand(command, args);
      return;
    }
  }

}

void RemoteCommands::processLine(const QString& line)
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
  RemoteCommands::processCommand(command, args);
}
