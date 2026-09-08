#include <QJsonObject>
#include "core/player.h"
#include "remotebasiccommands.h"
#include "remotejsoncreator.h"
#include "remotetypes.h"

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

void RemoteBasicCommands::remoteVolume(const QJsonObject& args){

    qint32 sentVol{ args[toString(Event::VOLUME)].toInt(-1) };
    if (sentVol == -1) wrongArgsSent(toString(Event::VOLUME));
    uint vol{static_cast<uint>(sentVol)};

    if (vol <=100) Q_EMIT volume(qBound(0u, vol, 100u));
}

void RemoteBasicCommands::remoteSeekTo(const QJsonObject& args){

    qint64 sentSeconds{ args[toString(Event::SEEK_TO)].toInteger(-1)};

    if (sentSeconds == -1) {
        wrongArgsSent(toString(Event::SEEK_TO));
        return;
    }
    Q_EMIT RemoteBasicCommands::seekTo(sentSeconds);
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
        field(Event::VOLUME, static_cast<int>(volume)),
    }));
}

void RemoteBasicCommands::wrongArgsSent(const QString& error){
    Q_EMIT sendResponse(
        RemoteJsonCreator::createResponse({
            field(MessageType::ERROR, toString(MessageType::ERROR)),
            field(Error::ERROR, toString(Error::WRONG_ARGUMENT_SENT)),
            field(Arguments::REQUIRED, error)
        })
    );
}

void RemoteBasicCommands::createCommandMap()
{
    // Basic audio playback funtions.
    commandMap[toString(Event::PLAY)] = [this](const auto&){Q_EMIT play(); commandResponse(toString(Event::PLAY));};
    commandMap[toString(Event::PLAY_PAUSE)] = [this](const auto&) {Q_EMIT playPause(); commandResponse(toString(Event::PLAY));};
    commandMap[toString(Event::PAUSE)] = [this](const auto&){Q_EMIT pause(); commandResponse(toString(Event::PAUSE));};
    commandMap[toString(Event::STOP)] = [this](const auto&){Q_EMIT stop(false); commandResponse(toString(Event::STOP));};
    commandMap[toString(Event::NEXT)] = [this](const auto&){Q_EMIT next(); commandResponse(toString(Event::NEXT));};
    commandMap[toString(Event::PREVIOUS)] = [this](const auto&){Q_EMIT previous(); commandResponse(toString(Event::PREVIOUS));};
    commandMap[toString(Event::STOP_AFTER_CURRENT)] = [this](const auto&){Q_EMIT stopAfterCurrent(); commandResponse(toString(Event::STOP));};
    commandMap[toString(Event::RESTART_OR_PREVIOUS)] = [this](const auto&){Q_EMIT restartOrPrevious(); commandResponse(toString(Event::PREVIOUS));};
    commandMap[toString(Event::VOLUME)] = [this](const QJsonObject& args){ remoteVolume(args);};
    commandMap[toString(Event::VOLUME_UP)] = [this](const auto&){ Q_EMIT volumeUp(); commandResponse(toString(Event::VOLUME_CHANGED));};
    commandMap[toString(Event::VOLUME_DOWN)] = [this](const auto&){ Q_EMIT volumeDown(); commandResponse(toString(Event::VOLUME_CHANGED));};
    commandMap[toString(Event::MUTE)] = [this](const auto&){ Q_EMIT mute(); commandResponse(toString(Event::VOLUME_CHANGED));};
    commandMap[toString(Event::SEEK_TO)] = [this](const QJsonObject& args){remoteSeekTo(args);};
    commandMap[toString(Event::SEEK_BACKWARD)] = [this](const auto&){ Q_EMIT RemoteBasicCommands::seekBackward(); commandResponse(toString(Event::SEEK_BACKWARD));};
    commandMap[toString(Event::SEEK_FORWARD)] = [this](const auto&){ Q_EMIT RemoteBasicCommands::seekForward(); commandResponse(toString(Event::SEEK_FORWARD));};
}
