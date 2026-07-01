#include <QJsonObject>
#include "remotebasiccommands.h"
#include "remotecontroller/remoteconstants.h"
#include "core/player.h"
#include "remotecontroller/remotejsoncreator.h"
#include "remotecontroller/remotetypes.h"

using namespace Qt::Literals::StringLiterals;
using namespace RemoteTypes;

RemoteBasicCommands::RemoteBasicCommands(const Application *app)
    : app_(app)
{
  // Fill the commandMap.
  RemoteBasicCommands::createCommandMap();

  // Make connects for requests from remote devices.
  QObject::connect(this, &RemoteBasicCommands::mute, &*app_->player(), &Player::Mute);
  QObject::connect(this, &RemoteBasicCommands::next , &*app_->player(), &Player::Next);
  QObject::connect(this, &RemoteBasicCommands::play , &*app_->player(), &Player::PlayPauseHelper);
  QObject::connect(this, &RemoteBasicCommands::playPause , &*app_->player(), &Player::PlayPauseHelper);
  QObject::connect(this, &RemoteBasicCommands::pause, &*app_->player(), &Player::PlayPauseHelper);
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
}

void RemoteBasicCommands::commandResponse(const QString& command){
    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::RESPONSE, toString(Response::RUNNING_COMMAND)),
        field(Arguments::COMMAND, command)
    }));
}

void RemoteBasicCommands::remoteVolume(const QStringList& args){
    if(args.size() != 1) wrongNumArgs(1);

    bool ok;
    quint32 vol = remoteconstants::parseUintArg(args, ok);
    if (ok && vol <= 100){
        Q_EMIT volume(qBound(0u, vol, 100u));
    }
}

void RemoteBasicCommands::remoteSeekTo(const QStringList& args){
    if(args.size() != 1) wrongNumArgs(1);

    bool ok;
    quint32 seconds = remoteconstants::parseUintArg(args, ok);
    if (ok) {
        Q_EMIT RemoteBasicCommands::seekTo(seconds);
    }
}

const BasicCmdMap& RemoteBasicCommands::sendCommandMap() const
{
  return commandMap;
}

void RemoteBasicCommands::volumeChanged(const uint volume)
{
    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::VOLUME_CHANGED)),
        field(Arguments::VOLUME, static_cast<int>(volume)),
    }));
}

void RemoteBasicCommands::wrongNumArgs(const int num){
    Q_EMIT sendResponse(
        RemoteJsonCreator::createResponse({
            field(MessageType::ERROR, toString(MessageType::ERROR)),
            field(Error::ERROR, toString(Error::WRONG_NUMBER_ARGUMENTS_PASSED)),
            field(Arguments::REQUIRED, num)
        })
    );
}

void RemoteBasicCommands::createCommandMap()
{
    // Basic audio playback funtions.
    commandMap[u"play"_s] = [this](const auto&){Q_EMIT play(); commandResponse(toString(Event::PLAY));};
    commandMap[u"play-pause"_s] = [this](const auto&) {Q_EMIT playPause(); commandResponse(toString(Event::PLAY));};
    commandMap[u"pause"_s] = [this](const auto&){Q_EMIT pause(); commandResponse(toString(Event::PAUSE));};
    commandMap[u"stop"_s] = [this](const auto&){Q_EMIT stop(false); commandResponse(toString(Event::STOP));};
    commandMap[u"next"_s] = [this](const auto&){Q_EMIT next(); commandResponse(toString(Event::NEXT));};
    commandMap[u"previous"_s] = [this](const auto&){Q_EMIT previous(); commandResponse(toString(Event::PREVIOUS));};
    commandMap[u"stop-after-current"_s] = [this](const auto&){Q_EMIT stopAfterCurrent(); commandResponse(toString(Event::STOP));};
    commandMap[u"restart-or-previous"_s] = [this](const auto&){Q_EMIT restartOrPrevious(); commandResponse(toString(Event::PREVIOUS));};
    commandMap[u"volume"_s] = [this](const QStringList& args){ remoteVolume(args);};
    commandMap[u"volume-up"_s] = [this](const auto&){ Q_EMIT volumeUp(); commandResponse(toString(Event::VOLUME_CHANGED));};
    commandMap[u"volume-down"_s] = [this](const auto&){ Q_EMIT volumeDown(); commandResponse(toString(Event::VOLUME_CHANGED));};
    commandMap[u"mute"_s] = [this](const auto&){ Q_EMIT mute(); commandResponse(toString(Event::VOLUME_CHANGED));};
    commandMap[u"seek-to"_s] = [this](const QStringList& args){remoteSeekTo(args);};
    commandMap[u"seek-backward"_s] = [this](const auto&){ Q_EMIT RemoteBasicCommands::seekBackward(); commandResponse(toString(Event::SEEK_BACKWARD));};
    commandMap[u"seek-forward"_s] = [this](const auto&){ Q_EMIT RemoteBasicCommands::seekForward(); commandResponse(toString(Event::SEEK_FORWARD));};
}
