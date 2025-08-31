#include "remotebasiccommands.h"
#include "core/player.h"
#include "core/logging.h"


RemoteBasicCommands::RemoteBasicCommands(Application *app)//, QObject *parent)
    : //QObject{parent},
      app_(app),
      commandMap(RemoteBasicCommands::createCommandMap())
{}

bool RemoteBasicCommands::sendCommand(const QString& command, [[maybe_unused]] const QStringList &args)
{
  // If the command is a basic command, run it, otherwise return false to check other command lists.
  if ( commandMap.contains(command)){
    commandMap[command](args);
    return true;
  }
  qLog(Debug) << "No matching command sent to RemoteBacisCommands::runCommand: " << command;
  return false;

}

QMap<QString, std::function<void(const QStringList&)>> RemoteBasicCommands::createCommandMap()
{
  // Lambda to check if args contains an int.
  auto parseUintArg = [](const QStringList& args, bool& ok)-> quint32 {
    if (args.isEmpty()){
      ok = false;
      return 0;
    }
    return args.first().toUInt(&ok);
  };

  QMap<QString, std::function<void(const QStringList&)>> commandMap;
  // Basic audio playback funtions.
  commandMap[QStringLiteral("play")] = [this](const auto&){ app_->player()->Play();};
  commandMap[QStringLiteral("play-pause")] = [this](const auto&) { app_->player()->PlayPauseHelper();};
  commandMap[QStringLiteral("pause")] = [this](const auto&){ app_->player()->Pause();};
  commandMap[QStringLiteral("stop")] = [this](const auto&){ app_->player()->Stop();};
  commandMap[QStringLiteral("next")] = [this](const auto&){ app_->player()->Next();};
  commandMap[QStringLiteral("previous")] = [this](const auto&){ app_->player()->Previous();};
  commandMap[QStringLiteral("stop-after-current")] = [this](const auto&){ app_->player()->StopAfterCurrent();};
  commandMap[QStringLiteral("restart-or-previous")] = [this](const auto&){ app_->player()->RestartOrPrevious();};

  // Basic volume changes.
  commandMap[QStringLiteral("volume")] = [this, parseUintArg](const QStringList& args){
    bool ok;
    quint32 vol = parseUintArg(args, ok);
    qDebug() << "Remote volume with value: "<< vol;
    if (ok) app_->player()->SetVolume(qBound(0u, vol, 100u));
  };
  commandMap[QStringLiteral("volume-up")] = [this](const auto&){ app_->player()->VolumeUp();};
  commandMap[QStringLiteral("volume-down")] = [this](const auto&){ app_->player()->VolumeDown();};

  commandMap[QStringLiteral("mute")] = [this](const auto&){ app_->player()->Mute();};

  // Basic seek commands.
  commandMap[QStringLiteral("seek-to")] = [this, parseUintArg](const QStringList& args){
    bool ok;
    quint32 seconds = parseUintArg(args, ok);
    if (ok) app_->player()->SeekTo(seconds);
  };
  // Need to calculate from/to amount.
  commandMap[QStringLiteral("seek-by")] = [this, parseUintArg](const QStringList& args){
  bool ok;
  quint32 seconds = parseUintArg(args, ok);
  if (ok) {
    qint64 current_pos_sec = app_->player()->engine()->position_nanosec() / 1000000000;
            app_->player()->SeekTo(current_pos_sec + seconds);
  }
  };

  // A couple of spares, since I would forget :)
  // commandMap[QStringLiteral("play")] = [this](){ app_->;};
  // commandMap[QStringLiteral("play")] = [this](){ app_->;};
  return commandMap;
}
