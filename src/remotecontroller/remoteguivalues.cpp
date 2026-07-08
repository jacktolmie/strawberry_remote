#include "remoteguivalues.h"
#include "core/player.h"
#include "constants/timeconstants.h"
#include "playlist/playlistmanager.h"
#include "remotejsoncreator.h"
#include "remoteplaylist.h"
#include "remotetypes.h"

using namespace Qt::Literals::StringLiterals;
using namespace RemoteTypes;

// RemoteGuiValues::RemoteGuiValues(const RemotePlaylist *remotePlaylist, const Application* app, QObject *parent)
RemoteGuiValues::RemoteGuiValues(const RemotePlaylist *remotePlaylist, const Application* app, QObject *parent)
  : QObject{parent},
    app_{app},
    remotePlaylist_{remotePlaylist}
{}

QJsonObject RemoteGuiValues::getUpdates() const{

  RemoteTypes::Arguments currentPlayState;

  switch(app_->player()->GetState()){
    case EngineBase::State::Empty:
    case EngineBase::State::Idle:
    case EngineBase::State::Error: {
      currentPlayState = RemoteTypes::Arguments::STOPPED;
      break;
    }
    case EngineBase::State::Paused: {
      currentPlayState = RemoteTypes::Arguments::PAUSED;
      break;
    }
    case EngineBase::State::Playing: {
      currentPlayState = RemoteTypes::Arguments::PLAYING;
      break;
    }
  };

  return RemoteJsonCreator::createResponse({
    field(MessageType::EVENT, toString(MessageType::EVENT)),
    field(Event::EVENT, toString(Event::GUI_UPDATES)),
    field(Arguments::VOLUME, static_cast<qint32>(app_->player()->GetVolume())),
    field(Arguments::TIME, app_->player()->engine()->position_nanosec() / kNsecPerMsec),
    field(Arguments::PLAYING, toString(currentPlayState)),
    field(Arguments::PLAYLISTS, remotePlaylist_->sendAllPlaylists()),
    field(Arguments::CURRENT_PLAYLIST, app_->playlist_manager()->current_id()),
    field(Arguments::CURRENT_SONG, app_->playlist_manager()->active() ? app_->playlist_manager()->active()->current_index().row(): -1),
    field(Arguments::ACTIVE_PLAYLIST, app_->playlist_manager()->active_id())
  });
}

QJsonObject RemoteGuiValues::triggerUpdate() const {
  return getUpdates();
}
