#include <QJsonObject>
#include "remotebasiccommands.h"
#include "constants/timeconstants.h"
#include "remotecontroller/remoteconstants.h"
#include "core/player.h"
#include "playlist/playlistmanager.h"
#include "remotecontroller/remotejsoncreator.h"

using namespace Qt::Literals::StringLiterals;

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
  QObject::connect(this, &RemoteBasicCommands::seekBackward , &*app_->player(), &Player::SeekBackward);
  QObject::connect(this, &RemoteBasicCommands::seekForward , &*app_->player(), &Player::SeekForward);
  QObject::connect(this, &RemoteBasicCommands::stop , &*app_->player(), &Player::Stop);
  QObject::connect(this, &RemoteBasicCommands::stopAfterCurrent , &*app_->player(), &Player::StopAfterCurrent);
  QObject::connect(this, &RemoteBasicCommands::volume , &*app_->player(), &Player::SetVolume);
  QObject::connect(this, &RemoteBasicCommands::volumeDown , &*app_->player(), &Player::VolumeDown);
  QObject::connect(this, &RemoteBasicCommands::volumeUp , &*app_->player(), &Player::VolumeUp);

  QObject::connect(&*app_->player(), &Player::VolumeChanged, this, &RemoteBasicCommands::volumeChanged);
  // QObject::connect(&*app_->playlist_manager(), &PlaylistManager::sendPlayCommand, this, &RemoteBasicCommands::playlistPlay);
// QObject::connect(this, &RemoteBasicCommands:: , &*app_->player(), &Player::);
}

void RemoteBasicCommands::volumeChanged(const uint volume)
{
  Q_EMIT RemoteBasicCommands::sendResponse(RemoteJsonCreator::createResponse({
    {u"event"_s, u"Volume Changed new function"_s},
    { u"value"_s,  QString::number(volume)}
  }));
}

BasicCmdMap& RemoteBasicCommands::sendCommandMap()
{
  return commandMap;
}

void RemoteBasicCommands::createCommandMap()
{
  // Lambda to check if args contains an int. Inside remoteconstants header.
  auto parseUintArg{remoteconstants::parseUintArg};

  // Basic audio playback funtions.
  commandMap[u"play"_s] = [this](const auto&){Q_EMIT RemoteBasicCommands::play(app_->player()->engine()->position_nanosec());};
  commandMap[u"play-pause"_s] = [this](const auto&) {Q_EMIT RemoteBasicCommands::playPause();};
  commandMap[u"pause"_s] = [this](const auto&){Q_EMIT RemoteBasicCommands::pause();};
  commandMap[u"stop"_s] = [this](const auto&){Q_EMIT RemoteBasicCommands::stop(false);};
  commandMap[u"next"_s] = [this](const auto&){Q_EMIT RemoteBasicCommands::next();};
  commandMap[u"previous"_s] = [this](const auto&){Q_EMIT RemoteBasicCommands::previous();};
  commandMap[u"stop-after-current"_s] = [this](const auto&){Q_EMIT RemoteBasicCommands::stopAfterCurrent();};
  commandMap[u"restart-or-previous"_s] = [this](const auto&){Q_EMIT RemoteBasicCommands::restartOrPrevious();};

  // Basic volume changes.
  commandMap[u"volume"_s] = [this, parseUintArg](const QStringList& args){
    bool ok;
    quint32 vol = parseUintArg(args, ok);
    qDebug() << "Remote volume with value: "<< vol;
    if (ok) Q_EMIT RemoteBasicCommands::volume(qBound(0u, vol, 100u));
  };
  commandMap[u"volume-up"_s] = [this](const auto&){ Q_EMIT RemoteBasicCommands::volumeUp();};
  commandMap[u"volume-down"_s] = [this](const auto&){ Q_EMIT RemoteBasicCommands::volumeDown();};
  // Delete when done testing song timer position
  commandMap[u"current"_s] = [this](const auto&){qDebug() << "Remote Current time: "<< app_->player()->engine()->position_nanosec() / kNsecPerMsec;};

  commandMap[u"mute"_s] = [this](const auto&){ Q_EMIT RemoteBasicCommands::mute();};

  // Basic seek commands.
  commandMap[u"seek-to"_s] = [this, parseUintArg](const QStringList& args){
    bool ok;
    quint32 seconds = parseUintArg(args, ok);
    if (ok) Q_EMIT RemoteBasicCommands::seekTo(seconds);
  };

  commandMap[u"seek-backward"_s] = [this](const auto&){ Q_EMIT RemoteBasicCommands::seekBackward();};
  commandMap[u"seek-forward"_s] = [this](const auto&){ Q_EMIT RemoteBasicCommands::seekForward();};

  // commandMap[u"play"_s)] = [this](const auto&){ app_->;};
}
