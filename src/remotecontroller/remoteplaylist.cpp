#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <qtpreprocessorsupport.h>

#include "constants/timeconstants.h"
#include "core/player.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlist.h"
#include "playlist/playlistsequence.h"

#include "remoteplaylist.h"
#include "remotejsoncreator.h"
#include "remotetypes.h"

using namespace Qt::Literals::StringLiterals;

using namespace RemoteTypes;

RemotePlaylist::RemotePlaylist(const Application *app, QObject *parent)
    : QObject{parent},
      app_(app),
      currentSong_(new RemoteCurrentSong(app_))
{
  RemotePlaylist::createCommandMap();

  QObject::connect(this, &RemotePlaylist::clearPlaylist , &*app_->playlist_manager(), &PlaylistManager::ClearCurrent);
  QObject::connect(this, &RemotePlaylist::remoteClosedPlaylist , &*app_->playlist_manager(), &PlaylistManager::Close);
  QObject::connect(this, &RemotePlaylist::deletePlaylist , &*app_->playlist_manager(), &PlaylistManager::Delete);
  QObject::connect(this, &RemotePlaylist::remoteSongSelected, &*app_->playlist_manager(), &PlaylistManager::PlayRequested);
  QObject::connect(this, &RemotePlaylist::remoteFavouritePlaylist , &*app_->playlist_manager(), &PlaylistManager::Favorite);
  QObject::connect(this, &RemotePlaylist::removeCurrentSong , &*app_->playlist_manager(), &PlaylistManager::RemoveCurrentSong);
  QObject::connect(this, &RemotePlaylist::removeDuplicates , &*app_->playlist_manager(), &PlaylistManager::RemoveDuplicatesCurrent);
  QObject::connect(this, &RemotePlaylist::removeUnavailable, &*app_->playlist_manager(), &PlaylistManager::RemoveUnavailableCurrent);
  QObject::connect(this, &RemotePlaylist::removeItemsWithoutUndo, &*app_->playlist_manager(), &PlaylistManager::RemoveItemsWithoutUndo);
  QObject::connect(this, &RemotePlaylist::remoteRenamePlaylist , &*app_->playlist_manager(), &PlaylistManager::Rename);
  QObject::connect(this, &RemotePlaylist::setCurrentPlaylistSignal , &*app_->playlist_manager(), &PlaylistManager::SetCurrentPlaylist);
  QObject::connect(this, &RemotePlaylist::shufflePlaylist , &*app_->playlist_manager(), &PlaylistManager::ShuffleCurrent);
  QObject::connect(this, &RemotePlaylist::serverFavouritePlaylist, this, &RemotePlaylist::favouriteServerPlaylist);
  QObject::connect(this, &RemotePlaylist::setActivePlaylist, &*app_->playlist_manager(), &PlaylistManager::SetActivePlaylist);
  QObject::connect(currentSong_, &RemoteCurrentSong::sendCurrentSongData, this, &RemotePlaylist::sendResponse);
  // QObject::connect(currentSong_, &RemoteCurrentSong::sendAlbumArt, this, &RemotePlaylist::sendResponse);
  QObject::connect(this, &RemotePlaylist::requestAlbumArt, currentSong_, &RemoteCurrentSong::requestAlbumArt);

  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::PlaylistClosed, this, &RemotePlaylist::closeServerPlaylist);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::PlaylistFavorited, this, &RemotePlaylist::serverFavouritePlaylist);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::PlaylistManagerInitialized, this, &RemotePlaylist::playlistManagerLoaded);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::renamePlaylist, this, &RemotePlaylist::serverRenamePlaylist);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::sendActivePlaylistId, this, &RemotePlaylist::activeChanged);

  metadataTimer_ = new QTimer(this);
  metadataTimer_->setSingleShot(true);
  metadataTimer_->setInterval(3000);
  connect(metadataTimer_, &QTimer::timeout, this, &RemotePlaylist::sendPendingPlaylist);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::PlaylistItemMetadataChanged, this, &RemotePlaylist::onPlaylistMetadataChangedWithQUuid);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::sendPlaylistToCreate, this, &RemotePlaylist::onPlaylistMetadataChanged);

  // Unused??
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::PlaylistItemsAdded, this, &RemotePlaylist::playlistItemsAdded);


  // QObject::connect(&*app_->playlist_manager(), &PlaylistManager::sendPlaylistToCreate, this, &RemotePlaylist::sendPlaylistData);
  // QObject::connect(this, &RemotePlaylist::deletePlaylist, this, &RemotePlaylist::deleteServerPlaylist);
  // QObject::connect(&*app_->playlist_manager(), &PlaylistManager::deletePlaylistId, this, &RemotePlaylist::deleteServerPlaylist);
  // QObject::connect(this, &RemotePlaylist::remoteClosedPlaylist, this, &RemotePlaylist::closeServerPlaylist);

}

// Unused???
void RemotePlaylist::playlistItemsAdded(const int playlist_id, const QList<QUuid> &track_ids, const QUuid after_track_id){
    qInfo()<<"Playlist item added: " << playlist_id;
    for(auto& id: track_ids) qInfo() << "Track ID: " << id;
}

void RemotePlaylist::onPlaylistMetadataChangedWithQUuid(const int id, const QUuid track_id = QUuid()) {
    Q_UNUSED(track_id);
    // pendingPlaylistId_ = id;
    // metadataTimer_->start();
    qInfo()<<"RemotePlaylist playlistMetadatachanged called with id: " << id;
    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::MAKE_PLAYLIST)),
        field(Arguments::PLAYLIST_ID, id),
        field(Arguments::PLAYLIST, makePlaylistData(id))
    }));

}

void RemotePlaylist::onPlaylistMetadataChanged(const int id) {
     onPlaylistMetadataChangedWithQUuid(id);
}

void RemotePlaylist::sendPendingPlaylist() {
    sendPlaylistData(pendingPlaylistId_);

}

void RemotePlaylist::activeChanged(const int id){

    Q_EMIT RemotePlaylist::sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::ACTIVE_PLAYLIST)),
        field(Arguments::ID, id),
        field(Arguments::ROW, app_->playlist_manager()->active() ? app_->playlist_manager()->active()->current_row() : -1)
    }));
}

QJsonObject RemotePlaylist::clearRemoteCurrentPlaylist(const QJsonObject& args){

    qint32 id{ args[u"id"_s].toInt(-1) };
    if (id == -1) return wrongArgsSent(u"id"_s); //(1);

    // Get current playlist id, set to remote current, clear it, then set it back.
    int serverCurrent{app_->playlist_manager()->current_id()};
    app_->playlist_manager()->SetCurrentPlaylist(id);
    app_->playlist_manager()->ClearCurrent();
    app_->playlist_manager()->SetCurrentPlaylist(serverCurrent);
    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::RESPONSE, toString(Response::CLEARED_PLAYLIST)),
    });
}

QJsonObject RemotePlaylist::closeRemoteCurrentPlaylist(const QJsonObject& args){

    qint32 id{ args[u"id"_s].toInt(-1) };
    if (id == -1) return wrongArgsSent(u"id"_s); //(1);

    int current{app_->playlist_manager()->current_id()};
    app_->playlist_manager()->SetCurrentPlaylist(id);
    Q_EMIT remoteClosedPlaylist(id);
    app_->playlist_manager()->SetCurrentPlaylist(current);

    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::RESPONSE, toString(Response::PLAYLIST_CLOSED))
    });
}

void RemotePlaylist::closeServerPlaylist(const int id){
    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::CLOSED_PLAYLIST_WITH_ID)),
        field(Arguments::ID, id)
    }));
}

QJsonObject RemotePlaylist::deleteCurrentRemotePlaylist(const QJsonObject& args){

    qint32 id{ args[u"id"_s].toInt(-1) };
    if (id == -1) return wrongArgsSent(u"id"_s); //(1);

    app_->playlist_manager()->Delete(id);

    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::RESPONSE, toString(Response::DELETED_PLAYLIST_WITH_ID)),
        field(Arguments::ID, id)
    });
}

void RemotePlaylist::deleteServerPlaylist(const int id){
    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::RESPONSE, toString(Response::DELETED_PLAYLIST_WITH_ID)),
        field(Arguments::ID, id)
    }));
}

void RemotePlaylist::favouriteServerPlaylist(const int id, bool favourite){
    Q_EMIT RemotePlaylist::sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::FAVOURITE_PLAYLIST)),
        field(Arguments::ID, id),
        field(Arguments::FAVOURITE, favourite)
    }));
}

QJsonObject RemotePlaylist::makeAllPlaylists() const{
    QJsonArray playlistArray;
    auto playlists{app_->playlist_manager()->GetAllPlaylists()};
    for (const auto& playlist: playlists){
        playlistArray.append(RemotePlaylist::makePlaylistData(playlist->id()));
    }
    return RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::MAKE_ALL_PLAYLISTS)),
        field(Arguments::PLAYLISTS, playlistArray)
    });
}

QJsonObject RemotePlaylist::makePlaylistData(const int id) const{
    qInfo()<<"makeplaylistdat called with id: " << id;
    QJsonObject playlistObject;
    playlistObject[toString(Arguments::NAME)] =             app_->playlist_manager()->playlist_name(id);
    playlistObject[toString(Arguments::ID)] =               id;
    playlistObject[toString(Arguments::FAVOURITE)] =        app_->playlist_manager()->IsPlaylistFavorite(id);
    playlistObject[toString(Arguments::PLAYLIST_LENGTH)] =  static_cast<qint64>(app_->playlist_manager()->playlist(id)->GetTotalLength() /kNsecPerMsec);
    playlistObject[toString(Arguments::PLAYLIST_SIZE)] =    app_->playlist_manager()->playlist(id)->rowCount();

    QJsonArray songsArray;
    auto songs{app_->playlist_manager()->playlist(id)->GetAllSongs()};
    RemoteCurrentSong songInfo = RemoteCurrentSong(app_);
    for(int index{0}; index < songs.size(); ++index){
        songsArray.append(songInfo.songData(songs[index], id, index));
    }

    playlistObject[toString(RemoteTypes::Arguments::SONGS)] = songsArray;
    qInfo()<<"Song array: " << playlistObject;
    return playlistObject;
}

void RemotePlaylist::playlistManagerLoaded(){
    QObject::connect(this, &RemotePlaylist::setRepeatModeSignal, app_->playlist_manager()->sequence(), &PlaylistSequence::SetRepeatMode);
    QObject::connect(app_->playlist_manager()->sequence(), &PlaylistSequence::RepeatModeChanged, this, &RemotePlaylist::repeatModeChanged);
}

QJsonObject RemotePlaylist::receiveRemoteActive(const QJsonObject& args){

    qint32 id{ args[u"id"_s].toInt(-1)};
    if (id == -1) return wrongArgsSent(u"id"_s);
    qint32 songIndex{ args[u"song_index"].toInt(-1) };
    if (songIndex == -1) return wrongArgsSent(u"song_index"_s);

    // Set remotes active playlist and song index on server.
    Q_EMIT RemotePlaylist::setActivePlaylist(id);
    app_->playlist_manager()->SetActivePlaylist(id);
    app_->playlist_manager()->active()->set_current_row(songIndex);
    Q_EMIT RemotePlaylist::setCurrentPlaylistSignal(id);

    auto currentIndex{app_->playlist_manager()->current()->current_index()};
    Q_EMIT remoteSongSelected(currentIndex, Playlist::AutoScroll::Maybe);

    return RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::SENT_ACTIVE_PLAYLIST)),
        field(Arguments::ID, static_cast<int>(id))
    });
}

QJsonObject RemotePlaylist::remoteChangedPlaylist(const QJsonObject& args){
    qint32 id{ args[u"id"_s].toInt(-1)};
    if (id == -1) return wrongArgsSent(u"id"_s);

    qint32 fromIndex{ args[u"from-index"_s].toInt(-1)};
    if (fromIndex == -1) return wrongArgsSent(u"from-index"_s);

    qint32 toIndex{ args[u"to-index"_s].toInt(-1)};
    if (toIndex == -1) return wrongArgsSent(u"to-index"_s);

    PlaylistItemPtrList newItems = app_->playlist_manager()->current()->GetAllItems();
    qInfo() << "remoteplaylist current playist:";
    for(auto song: newItems) qInfo()<< song->EffectiveMetadata().PrettyTitle();

    auto moved{app_->playlist_manager()->current()->item_at(fromIndex)};
    newItems.removeAt(fromIndex);
    newItems.insert(toIndex, moved);
    qInfo() << "remoteplaylist current playist after change:";
    for(auto song: newItems) qInfo()<< song->EffectiveMetadata().PrettyTitle();
    app_->playlist_manager()->current()->receiveChangedPlaylist(newItems);

    return makePlaylistData(id);
}

QJsonObject RemotePlaylist::removeCurrentSongsPlaylist(const QJsonObject& args){

    qint32 id{ args[u"id"_s].toInt(-1)};
    if (id == -1) return wrongArgsSent(u"id"_s);

    // Check if the argument has a list of songs to be removed.
    if (!args.contains(u"songs_list"_s) || !args[u"songs_list"_s].isArray() ){
        return wrongArgsSent(u"songs_list"_s);
    }

    QJsonArray songsArray = args[u"songs_list"_s].toArray(); // Needs =, not {} or creates wrong array!!!

    QList<int> songList;
    songList.reserve(songsArray.size());

    for(auto& song: std::as_const(songsArray)){
        songList.emplaceBack(song.toInt());
    }

    Q_EMIT removeItemsWithoutUndo(id, songList);

    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::RESPONSE, toString(Response::REMOVED_SONGS_FROM_PLAYLIST)),
        field(Arguments::ID, id)
    });
}

QJsonObject RemotePlaylist::removeDuplicatesPlaylist(const QJsonObject& args){

    qint32 id{args[u"id"_s].toInt(-1)};
    if (id == -1) return wrongArgsSent(u"id"_s);

    int current{app_->playlist_manager()->current_id()};
    app_->playlist_manager()->SetCurrentPlaylist(id);
    Q_EMIT RemotePlaylist::removeDuplicates();
    app_->playlist_manager()->SetCurrentPlaylist(current);

    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::REMOVED_DUPLICATES_FROM_PLAYLIST, toString(Response::REMOVED_DUPLICATES_FROM_PLAYLIST))
    });
}

QJsonObject RemotePlaylist::removeUnavailableSongs(const QJsonObject& args){
    qint32 id{args[u"id"_s].toInt(-1)};
    if (id == -1) return wrongArgsSent(u"id"_s);

    int current{app_->playlist_manager()->current_id()};
    app_->playlist_manager()->SetCurrentPlaylist(id);
    Q_EMIT RemotePlaylist::removeUnavailable();
    app_->playlist_manager()->SetCurrentPlaylist(current);

    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::REMOVED_UNAVAILABLE_SONGS, toString(Response::REMOVED_UNAVAILABLE_SONGS))
    });
}

QJsonObject RemotePlaylist::renameCurrentPlaylist(const QJsonObject& args){

    if (!args.contains(u"name"_s)) return wrongArgsSent(u"name"_s);

    qint32 id{ args[u"id"_s].toInt(-1) };
    if (id == -1) return wrongArgsSent(u"id"_s);

    QString name{ args[u"name"_s].toString()};

    Q_EMIT remoteRenamePlaylist(id, name);

    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::RESPONSE, toString(Response::RENAME_PLAYLIST)),
        field(Arguments::NAME, app_->playlist_manager()->GetPlaylistName(id))
    });
}

QString RemotePlaylist::repeatMode() const{

    QString mode;

    switch (app_->playlist_manager()->sequence()->repeat_mode()) {
    case PlaylistSequence::RepeatMode::Album: {
        mode = u"album"_s;
        break;
    }
    case PlaylistSequence::RepeatMode::Off: {
        mode = u"off"_s;
        break;
    }
    case PlaylistSequence::RepeatMode::Playlist: {
        mode = u"playlist"_s;
        break;
    }
    case PlaylistSequence::RepeatMode::Track: {
        mode = u"track"_s;
        break;
    }
    default:
        break;
    }
    return mode;
}

void RemotePlaylist::repeatModeChanged([[ maybe_unused ]] const PlaylistSequence::RepeatMode mode){
    Q_EMIT sendResponse( RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::REPEAT_MODE)),
        field(Arguments::ID, app_->playlist_manager()->active_id()),
        field(Arguments::REPEAT_MODE, repeatMode())
    }));
}

QJsonObject RemotePlaylist::sendAllPlaylists() const {
    return makeAllPlaylists();
}

const PlaylistCmdMap& RemotePlaylist::sendCommandMap() const{
    return commandMap;
}

QJsonObject RemotePlaylist::sendCoverImage(const QJsonObject& args){

    if (!args.contains(u"cover_art"_s)) return wrongArgsSent(u"name"_s);

    Q_EMIT sendResponse(currentSong_->requestAlbumArt(args[u"cover_art"_s].toString()));

    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::RESPONSE, toString(Response::SENT_ALBUM_COVER))
    });
}

void RemotePlaylist::sendPlaylistData(const int id){
    Q_EMIT RemotePlaylist::sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::MAKE_PLAYLIST)),
        field(Arguments::ID, id),
        field(Arguments::PLAYLIST, makePlaylistData(id))
    }));
}

QJsonObject RemotePlaylist::sendRequestedPlaylist(const QJsonObject& args){

    qint32 id{ args[u"id"_s].toInt(-1) };
    if (id == -1) return wrongArgsSent(u"id"_s);

    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::RESPONSE, toString(Response::SEND_REQUESTED_PLAYLIST)),
        field(Arguments::PLAYLIST, makePlaylistData(id))
    });
}

void RemotePlaylist::serverRenamePlaylist(const int id, const QString& name){
    Q_EMIT RemotePlaylist::sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::RENAME_PLAYLIST)),
        field(Arguments::ID, id),
        field(Arguments::NAME, name)
    }));
}

QJsonObject RemotePlaylist::setCurrentPlaylist(const QJsonObject& args){

    qint32 id{ args[u"id"_s].toInt(-1) };
    if (id == -1) return wrongArgsSent(u"id"_s);

    Q_EMIT RemotePlaylist::setCurrentPlaylistSignal(id);
    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::RESPONSE, toString(Response::SET_CURRENT_PLAYLIST_TO)),
        field(Arguments::NAME, app_->playlist_manager()->current()->objectName())
    });
}

QJsonObject RemotePlaylist::setFavouritePlaylist(const QJsonObject& args){

    if (!args.contains(u"favourite"_s)) return wrongArgsSent(u"favourite"_s);
    bool favourite{ args[u"favourite"_s].toBool()};

    qint32 id{ args[u"id"_s].toInt(-1)};
    if (id == -1) return wrongArgsSent(u"id"_s);

    Q_EMIT RemotePlaylist::remoteFavouritePlaylist(id, favourite);
    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::RESPONSE, toString(Response::IS_PLAYLIST_A_FAVOURITE)),
        field(Arguments::IS_FAVOURITE, app_->playlist_manager()->playlist(id)->is_favorite())
    });
}

QJsonObject RemotePlaylist::setRepeatMode(const QJsonObject& args){

    if (!args.contains(u"repeat-mode"_s)) return wrongArgsSent(u"repeat-mode"_s);

    QString mode{ args[u"repeat-mode"_s].toString().toLower()};
    PlaylistSequence::RepeatMode sendMode{};

    if(mode == u"album"_s) sendMode = PlaylistSequence::RepeatMode::Album;
    else if(mode == u"off"_s) sendMode = PlaylistSequence::RepeatMode::Off;
    else if(mode == u"playlist"_s) sendMode = PlaylistSequence::RepeatMode::Playlist;
    else if(mode == u"track"_s) sendMode = PlaylistSequence::RepeatMode::Track;

    Q_EMIT setRepeatModeSignal(sendMode);
    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::RESPONSE, toString(Response::REPEAT_MODE)),
        field(Arguments::REPEAT_MODE, repeatMode())
    });
}

QJsonObject RemotePlaylist::shuffleAllPlaylists(){
  int currentId{app_->playlist_manager()->current_id()};

  QList<int> playlistIds{app_->playlist_manager()->playlist_ids()};

  for(auto& list: playlistIds){
    Q_EMIT RemotePlaylist::setCurrentPlaylistSignal(list);
    Q_EMIT RemotePlaylist::shufflePlaylist();
  }

  Q_EMIT RemotePlaylist::setCurrentPlaylistSignal(currentId);
  return RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::SHUFFLED_ALL_PLAYLISTS))
  });
}

QJsonObject RemotePlaylist::shuffleSinglePlaylist(const QJsonObject& args){

    qint32 id{ args[u"id"_s].toInt(-1) };
    if (id == -1) return wrongArgsSent(u"id"_s);

    int serverCurrent{app_->playlist_manager()->current_id()};
    app_->playlist_manager()->SetCurrentPlaylist(id);
    Q_EMIT RemotePlaylist::shufflePlaylist();
    app_->playlist_manager()->SetCurrentPlaylist(serverCurrent);

    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::SHUFFLED_PLAYLIST, toString(Response::SHUFFLED_PLAYLIST))
    });
}

QJsonObject RemotePlaylist::wrongArgsSent(const QString& error){
    return RemoteJsonCreator::createResponse({
        field(MessageType::ERROR, toString(MessageType::ERROR)),
        field(Error::ERROR, toString(Error::WRONG_ARGUMENT_SENT)),
        field(Arguments::REQUIRED, error)
    });
}

void RemotePlaylist::createCommandMap(){
    commandMap[u"clear-playlist"_s] = [this](const QJsonObject& args){ return clearRemoteCurrentPlaylist(args); };
    commandMap[u"close-playlist"_s] = [this](const QJsonObject& args){ return closeRemoteCurrentPlaylist(args); };
    commandMap[u"delete-playlist"_s] = [this](const QJsonObject& args){ return deleteCurrentRemotePlaylist(args); };
    commandMap[u"favourite-playlist"_s] = [this](const QJsonObject& args){ return setFavouritePlaylist(args); };
    commandMap[u"remote-changed-playlist"_s] = [this](const QJsonObject& args){ return remoteChangedPlaylist(args);};
    commandMap[u"remove-duplicates-playlist"_s] = [this](const QJsonObject& args){ return removeDuplicatesPlaylist(args); };
    commandMap[u"remove-unavailable-songs"_s] = [this](const QJsonObject& args){ return removeUnavailableSongs(args); };
    commandMap[u"remove-songs-playlist"_s] = [this](const QJsonObject& args){ return removeCurrentSongsPlaylist(args); };
    commandMap[u"rename-playlist"_s] = [this](const QJsonObject& args){ return renameCurrentPlaylist(args); };
    commandMap[u"repeat-mode"_s] = [this](const QJsonObject& args){ return setRepeatMode(args);};
    commandMap[u"send-playlist-song"_s] = [this](const QJsonObject& args){ return receiveRemoteActive(args); };
    commandMap[u"send-all-playlists"_s] = [this](const auto&){ return makeAllPlaylists(); };
    commandMap[u"send-playlist"_s] = [this](const QJsonObject& args){ return sendRequestedPlaylist(args); };
    commandMap[u"request-cover"_s] = [this](const QJsonObject& args){ return sendCoverImage(args); };
    commandMap[u"set-current-playlist"_s] = [this](const QJsonObject& args){ return setCurrentPlaylist(args); };
    commandMap[u"shuffle-all-playlists"_s] = [this](const auto&){ return shuffleAllPlaylists(); };
    commandMap[u"shuffle-current-playlist"_s] = [this](const QJsonObject& args){ return shuffleSinglePlaylist(args); };
}