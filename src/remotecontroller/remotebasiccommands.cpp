#include <QJsonObject>
#include "remotebasiccommands.h"
#include "remotecontroller/remoteconstants.h"
#include "core/player.h"
#include "core/logging.h"

RemoteBasicCommands::RemoteBasicCommands(Application *app)
    : app_(app)
{
  // Fill the commandMap.
  RemoteBasicCommands::createCommandMap();

  // Make connects for sending to server.
  QObject::connect(this, &RemoteBasicCommands::mute, &*app_->player(), &Player::Mute);
  QObject::connect(this, &RemoteBasicCommands::next , &*app_->player(), &Player::Next);
  QObject::connect(this, &RemoteBasicCommands::play , &*app_->player(), &Player::Play);
  QObject::connect(this, &RemoteBasicCommands::playPause , &*app_->player(), &Player::PlayPauseHelper);
  QObject::connect(this, &RemoteBasicCommands::pause, &*app_->player(), &Player::Pause);
  QObject::connect(this, &RemoteBasicCommands::previous , &*app_->player(), &Player::Previous);
  QObject::connect(this, &RemoteBasicCommands::restartOrPrevious , &*app_->player(), &Player::RestartOrPrevious);
  QObject::connect(this, &RemoteBasicCommands::seekTo , &*app_->player(), &Player::SeekTo);
  // QObject::connect(this, &RemoteBasicCommands::seekBy , &*app_->player(), &Player::SeekTo);
  QObject::connect(this, &RemoteBasicCommands::seekBackward , &*app_->player(), &Player::SeekBackward);
  QObject::connect(this, &RemoteBasicCommands::seekForward , &*app_->player(), &Player::SeekForward);
  QObject::connect(this, &RemoteBasicCommands::stop , &*app_->player(), &Player::Stop);
  QObject::connect(this, &RemoteBasicCommands::stopAfterCurrent , &*app_->player(), &Player::StopAfterCurrent);
  QObject::connect(this, &RemoteBasicCommands::volume , &*app_->player(), &Player::SetVolume);
  QObject::connect(this, &RemoteBasicCommands::volumeDown , &*app_->player(), &Player::VolumeDown);
  QObject::connect(this, &RemoteBasicCommands::volumeUp , &*app_->player(), &Player::VolumeUp);

  QObject::connect(&*app_->player(), &Player::VolumeChanged, this, &RemoteBasicCommands::volumeChanged);

// QObject::connect(this, &RemoteBasicCommands:: , &*app_->player(), &Player::);
// QObject::connect(this, &RemoteBasicCommands:: , &*app_->player(), &Player::);
// QObject::connect(this, &RemoteBasicCommands:: , &*app_->player(), &Player::);
// QObject::connect(this, &RemoteBasicCommands:: , &*app_->player(), &Player::);
// QObject::connect(this, &RemoteBasicCommands:: , &*app_->player(), &Player::);
}


QJsonObject RemoteBasicCommands::checkCommand(const QString& command, const QStringList &args)
{
  // If the command is a basic command, run it
  if ( commandMap.contains(command)){
    commandMap[command](args);
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Running command: %1").arg(command)}};
  }
  return QJsonObject{{QStringLiteral("response"), QStringLiteral("Command '%1' not found").arg(command)}};
}

void RemoteBasicCommands::volumeChanged(const uint volume)
{
  QJsonObject response;
  response[QStringLiteral("event")] = QStringLiteral("volumeChanged");
  response[QStringLiteral("volume")] = static_cast<int>(volume);
  Q_EMIT RemoteBasicCommands::sendResponse(response);

}

CommandsMap& RemoteBasicCommands::sendCommandMap()
{
  return commandMap;
}

// void RemoteBasicCommands::sendResponse(QJsonObject response)
// {
//   // Delete function if not used.
// }

void RemoteBasicCommands::createCommandMap()
{
  // Lambda to check if args contains an int. Inside remoteconstants header.
  auto parseUintArg{remoteconstants::parseUintArg};

  // Basic audio playback funtions.
  commandMap[QStringLiteral("play")] = [this](const auto&){Q_EMIT RemoteBasicCommands::play(app_->player()->engine()->position_nanosec());};
  commandMap[QStringLiteral("play-pause")] = [this](const auto&) {Q_EMIT RemoteBasicCommands::playPause();};
  commandMap[QStringLiteral("pause")] = [this](const auto&){Q_EMIT RemoteBasicCommands::pause();};
  commandMap[QStringLiteral("stop")] = [this](const auto&){Q_EMIT RemoteBasicCommands::stop(false);};
  commandMap[QStringLiteral("next")] = [this](const auto&){Q_EMIT RemoteBasicCommands::next();};
  commandMap[QStringLiteral("previous")] = [this](const auto&){Q_EMIT RemoteBasicCommands::previous();};
  commandMap[QStringLiteral("stop-after-current")] = [this](const auto&){Q_EMIT RemoteBasicCommands::stopAfterCurrent();};
  commandMap[QStringLiteral("restart-or-previous")] = [this](const auto&){Q_EMIT RemoteBasicCommands::restartOrPrevious();};

  // Basic volume changes.
  commandMap[QStringLiteral("volume")] = [this, parseUintArg](const QStringList& args){
    bool ok;
    quint32 vol = parseUintArg(args, ok);
    qDebug() << "Remote volume with value: "<< vol;
    if (ok) Q_EMIT RemoteBasicCommands::volume(qBound(0u, vol, 100u));
  };
  commandMap[QStringLiteral("volume-up")] = [this](const auto&){ Q_EMIT RemoteBasicCommands::volumeUp();};
  commandMap[QStringLiteral("volume-down")] = [this](const auto&){ Q_EMIT RemoteBasicCommands::volumeDown();};
  // Delete when done testing song timer position
  commandMap[QStringLiteral("current")] = [this](const auto&){qDebug() << "Remote Current time: "<< app_->player()->engine()->position_nanosec() / 1000000LL;};

  commandMap[QStringLiteral("mute")] = [this](const auto&){ Q_EMIT RemoteBasicCommands::mute();};

  // Basic seek commands.
  commandMap[QStringLiteral("seek-to")] = [this, parseUintArg](const QStringList& args){
    bool ok;
    quint32 seconds = parseUintArg(args, ok);
    if (ok) Q_EMIT RemoteBasicCommands::seekTo(seconds);
  };

  commandMap[QStringLiteral("seek-backward")] = [this](const auto&){ Q_EMIT RemoteBasicCommands::seekBackward();};
  commandMap[QStringLiteral("seek-forward")] = [this](const auto&){ Q_EMIT RemoteBasicCommands::seekForward();};

  // A couple of spares, since I would forget :)
  // commandMap[QStringLiteral("play")] = [this](const auto&){ app_->;};
  // commandMap[QStringLiteral("play")] = [this](const auto&){ app_->;};
}
