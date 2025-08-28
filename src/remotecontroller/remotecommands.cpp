#include "remotecommands.h"


RemoteCommands::RemoteCommands(Application* app, QObject* parent = nullptr):
  app_(app)
{
  qDebug() <<"RemoteCommands instantiated";

}

QString RemoteCommands::processCommand(const QString& command)
{
  qDebug() << "RemoteCommands::processCommand called with: "<< command;
  return command;
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

  command = obj[command].toString();

  QStringList args;
  QString value{QStringLiteral("value")};
  QString arg{QStringLiteral("args")};

  // If not a basic command, process it accordingly.
  if (obj.contains(value)) {
    // Handles simple cases like { "command": "volume", "value": 75 }
    args.append(QJsonValueRef(obj[value]).toVariant().toString());
  }
  else if (obj.contains(arg) && obj[arg].isArray()) {
    // Handles more complex cases like { "command": "add", "args": ["url1", "url2"] }
    QJsonArray argArray{obj[arg].toArray()};

    for(const QJsonValue& val : argArray) {
      args.append(val.toString())  ;
    }
  }

  // If the command is a basic command, emit to MainWindow.
  if (commands.contains(command)){
    Q_EMIT RemoteCommands::forwardToPlayer(command, args);
  }

  // Q_EMIT RemoteCommands::forwardToPlayer(command, args); // Figure out what to emit.
}
