#include "remoteguivalues.h"
#include "core/player.h"
#include "constants/timeconstants.h"
#include "playlist/playlistmanager.h"
#include "remotejsoncreator.h"
#include "remoteplaylist.h"
#include "remotetypes.h"

#include "collection/collectionmodel.h"

using namespace Qt::Literals::StringLiterals;
using namespace RemoteTypes;

RemoteGuiValues::RemoteGuiValues(const RemotePlaylist *remotePlaylist, const Application* app, QObject *parent)
  : QObject{parent},
    app_{app},
    remotePlaylist_{remotePlaylist}
{
    QObject::connect(&*app_->collection_model(), &CollectionModel::TotalAlbumCountUpdated, this, &RemoteGuiValues::musicTotals);
    QObject::connect(&*app_->collection_model(), &CollectionModel::TotalArtistCountUpdated, this, &RemoteGuiValues::musicTotals);
    QObject::connect(&*app_->collection_model(), &CollectionModel::TotalSongCountUpdated, this, &RemoteGuiValues::musicTotals);
}

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

    // Get cover image data
    const Song& metadata = app_->playlist_manager()->current()->current_item_metadata();
    QByteArray imageData;
    QFile file(metadata.art_manual().toLocalFile());
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        imageData = file.readAll();
        file.close();
    } else {
        QFile autoFile(metadata.art_automatic().toLocalFile());
        if (autoFile.exists() && autoFile.open(QIODevice::ReadOnly)) {
            imageData = autoFile.readAll();
            autoFile.close();
        }
    }

    return RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::GUI_UPDATES)),
        field(PlaylistData::ACTIVE_PLAYLIST, app_->playlist_manager()->active_id()),
        field(Arguments::COVER_IMAGE, QString::fromLatin1(imageData.toBase64())),
        field(PlaylistData::CURRENT_PLAYLIST, app_->playlist_manager()->current_id()),
        field(Arguments::CURRENT_SONG, app_->playlist_manager()->active() ? app_->playlist_manager()->active()->current_index().row(): -1),
        field(Arguments::PLAYING, toString(currentPlayState)),
        field(PlaylistData::PLAYLISTS, remotePlaylist_->sendAllPlaylists()),
        field(PlaylistData::REPEAT_MODE, remotePlaylist_->repeatMode(app_->playlist_manager()->sequence()->repeat_mode())),
        field(PlaylistData::SHUFFLE_MODE, remotePlaylist_->shuffleMode(app_->playlist_manager()->sequence()->shuffle_mode())),
        field(Arguments::TIME, app_->player()->engine()->position_nanosec() / kNsecPerMsec),
        field(PlaylistData::TOTAL_ALBUMS, app_->collection_model()->total_album_count()),
        field(PlaylistData::TOTAL_ARTISTS, app_->collection_model()->total_artist_count()),
        field(PlaylistData::TOTAL_SONGS, app_->collection_model()->total_song_count()),
        field(Event::VOLUME, static_cast<qint32>(app_->player()->GetVolume()))
    });
}
// QJsonObject RemoteGuiValues::getUpdates() const{
//   RemoteTypes::Arguments currentPlayState;

//   switch(app_->player()->GetState()){
//     case EngineBase::State::Empty:
//     case EngineBase::State::Idle:
//     case EngineBase::State::Error: {
//       currentPlayState = RemoteTypes::Arguments::STOPPED;
//       break;
//     }
//     case EngineBase::State::Paused: {
//       currentPlayState = RemoteTypes::Arguments::PAUSED;
//       break;
//     }
//     case EngineBase::State::Playing: {
//       currentPlayState = RemoteTypes::Arguments::PLAYING;
//       break;
//     }
//   };

//   return RemoteJsonCreator::createResponse({
//     field(MessageType::EVENT, toString(MessageType::EVENT)),
//     field(Event::EVENT, toString(Event::GUI_UPDATES)),
//     field(Arguments::ACTIVE_PLAYLIST, app_->playlist_manager()->active_id()),
//     field(Arguments::COVER_IMAGE, QFileInfo(app_->playlist_manager()->current()->current_item_metadata().art_manual().toLocalFile()).fileName()),
//     field(Arguments::CURRENT_PLAYLIST, app_->playlist_manager()->current_id()),
//     field(Arguments::CURRENT_SONG, app_->playlist_manager()->active() ? app_->playlist_manager()->active()->current_index().row(): -1),
//     field(Arguments::PLAYING, toString(currentPlayState)),
//     field(Arguments::PLAYLISTS, remotePlaylist_->sendAllPlaylists()),
//     field(Arguments::REPEAT_MODE, remotePlaylist_->repeatMode(app_->playlist_manager()->sequence()->repeat_mode())),
//     field(Arguments::SHUFFLE_MODE, remotePlaylist_->shuffleMode(app_->playlist_manager()->sequence()->shuffle_mode())),
//     field(Arguments::TIME, app_->player()->engine()->position_nanosec() / kNsecPerMsec),
//     field(Arguments::TOTAL_ALBUMS, app_->collection_model()->total_album_count()),
//     field(Arguments::TOTAL_ARTISTS, app_->collection_model()->total_artist_count()),
//     field(Arguments::TOTAL_SONGS, app_->collection_model()->total_song_count()),
//     field(Arguments::VOLUME, static_cast<qint32>(app_->player()->GetVolume()))
//   });
// }

QJsonObject RemoteGuiValues::triggerUpdate() const {
  return getUpdates();
}

void RemoteGuiValues::musicTotals([[ maybe_unused ]] const int count){
    Q_EMIT sendCurrentStatus(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::MUSIC_TOTALS)),
        field(PlaylistData::TOTAL_ALBUMS, app_->collection_model()->total_album_count()),
        field(PlaylistData::TOTAL_ARTISTS, app_->collection_model()->total_artist_count()),
        field(PlaylistData::TOTAL_SONGS, app_->collection_model()->total_song_count())
    }));
}
