#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <qtpreprocessorsupport.h>
#include "remoteplaylist.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistbackend.h"
#include "playlist/playlist.h"
#include "core/player.h"
#include "remotecontroller/remoteconstants.h"
#include "remotecontroller/remotejsoncreator.h"
#include "remotecontroller/remotetypes.h"

using namespace Qt::Literals::StringLiterals;

using namespace RemoteTypes;

RemotePlaylist::RemotePlaylist(const Application *app, QObject *parent)
    : QObject{parent},
      app_(app),
      currentSong_(new RemoteCurrentSong(app_))
{
  RemotePlaylist::createCommandMap();

  // Might have double connections for favourite/closing/deleting playlists. Test with both remote and server tests.
  QObject::connect(this, &RemotePlaylist::clearPlaylist , &*app_->playlist_manager(), &PlaylistManager::ClearCurrent);
  QObject::connect(this, &RemotePlaylist::closePlaylist , &*app_->playlist_manager(), &PlaylistManager::Close);
  QObject::connect(this, &RemotePlaylist::deletePlaylist , &*app_->playlist_manager(), &PlaylistManager::Delete);
  QObject::connect(this, &RemotePlaylist::remoteFavouritePlaylist , &*app_->playlist_manager(), &PlaylistManager::Favorite);
  QObject::connect(this, &RemotePlaylist::removeCurrentSong , &*app_->playlist_manager(), &PlaylistManager::RemoveCurrentSong);
  QObject::connect(this, &RemotePlaylist::removeDuplicates , &*app_->playlist_manager(), &PlaylistManager::RemoveDuplicatesCurrent);
  QObject::connect(this, &RemotePlaylist::removeItemsWithoutUndo, &*app_->playlist_manager(), &PlaylistManager::RemoveItemsWithoutUndo);
  QObject::connect(this, &RemotePlaylist::remoteRenamePlaylist , &*app_->playlist_manager(), &PlaylistManager::Rename);
  QObject::connect(this, &RemotePlaylist::setCurrentPlaylistSignal , &*app_->playlist_manager(), &PlaylistManager::SetCurrentPlaylist);
  QObject::connect(this, &RemotePlaylist::shufflePlaylist , &*app_->playlist_manager(), &PlaylistManager::ShuffleCurrent);
  QObject::connect(this, &RemotePlaylist::deletePlaylist, this, &RemotePlaylist::deleteServerPlaylist);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::deletePlaylistId, this, &RemotePlaylist::deleteServerPlaylist);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::PlaylistClosed, this, &RemotePlaylist::closePlaylist);
  QObject::connect(this, &RemotePlaylist::closePlaylist, this, &RemotePlaylist::closeServerPlaylist);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::PlaylistFavorited, this, &RemotePlaylist::serverFavouritePlaylist);
  QObject::connect(this, &RemotePlaylist::serverFavouritePlaylist, this, &RemotePlaylist::favouriteServerPlaylist);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::renamePlaylist, this, &RemotePlaylist::serverRenamePlaylist);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::sendActivePlaylistId, this, &RemotePlaylist::activeChanged);
  QObject::connect(this, &RemotePlaylist::setActivePlaylist, &*app_->playlist_manager(), &PlaylistManager::SetActivePlaylist);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::sendPlaylistToCreate, this, &RemotePlaylist::sendPlaylistData);
  QObject::connect(currentSong_, &RemoteCurrentSong::sendCurrentSongData, this, &RemotePlaylist::sendResponse);
  QObject::connect(currentSong_, &RemoteCurrentSong::sendAlbumArt, this, &RemotePlaylist::sendResponse);
}

void RemotePlaylist::activeChanged(const int id){
    Q_EMIT RemotePlaylist::sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::ACTIVE_PLAYLIST)),
        field(Arguments::ID, id),
        field(Arguments::ROW, app_->playlist_manager()->active() ? app_->playlist_manager()->active()->current_row() : -1)
    }));
}

QJsonObject RemotePlaylist::clearRemoteCurrentPlaylist(const QStringList& args){
    if(args.size() != 1) return wrongNumArgs(1);

    bool ok;
    quint32 id{remoteconstants::parseUintArg(args, ok)};
    if(ok){
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
    else return RemoteJsonCreator::createResponse({
            field(MessageType::ERROR, toString(MessageType::ERROR)),
            field(Error::ERROR, toString(Error::PLAYLIST_NOT_CLEARED))
        });
}

QJsonObject RemotePlaylist::closeCurrentPlaylist(const QStringList& args){
    if(args.size() != 1) return wrongNumArgs(1);

    bool ok;
    quint32 id{remoteconstants::parseUintArg(args, ok)};
    if(ok){
        int current{app_->playlist_manager()->current_id()};
        Q_EMIT closePlaylist(id);
        if(current != app_->playlist_manager()->current_id()) return RemoteJsonCreator::createResponse({
                field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
                field(Response::RESPONSE, toString(Response::PLAYLIST_CLOSED))
            });
        else return RemoteJsonCreator::createResponse({
                field(MessageType::ERROR, toString(MessageType::ERROR)),
                field(Error::ERROR, toString(Error::PLAYLIST_NOT_CLOSED))
            });
    }
    return RemoteJsonCreator::createResponse({
        field(MessageType::ERROR, toString(MessageType::ERROR)),
        field(Error::ERROR, toString(Error::WRONG_ARGUMENT_SENT)),
        field(Arguments::ARGUMENT, args.first())
    });
}

void RemotePlaylist::closeServerPlaylist(const int id){
    Q_EMIT RemotePlaylist::sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::CLOSED_PLAYLIST_WITH_ID)),
        field(Arguments::ID, id)
    }));
}

QJsonObject RemotePlaylist::deleteCurrentDevicePlaylist(const QStringList& args){
    if(args.size() != 1) return wrongNumArgs(1);

    QString name{u"No name"_s};
    bool ok;
    quint32 id{remoteconstants::parseUintArg(args, ok)};
    if(ok){
        name = {app_->playlist_backend()->GetPlaylist(id).name};
        Q_EMIT RemotePlaylist::deletePlaylist(id);
        return RemoteJsonCreator::createResponse({
            field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
            field(Response::RESPONSE, toString(Response::DELETED_PLAYLIST_WITH_ID)),
            field(Arguments::NAME, name)
        });
    }
    return RemoteJsonCreator::createResponse({
        field(MessageType::ERROR, toString(MessageType::ERROR)),
        field(Error::ERROR, toString(Error::PLAYLIST_NOT_FOUND)),
        field(Arguments::NAME, name)
    });
}

void RemotePlaylist::deleteServerPlaylist(const int id){
    Q_EMIT RemotePlaylist::sendResponse(RemoteJsonCreator::createResponse({
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
    QJsonObject playlistObject;
    playlistObject[toString(Arguments::NAME)] = app_->playlist_manager()->playlist_name(id);
    playlistObject[toString(Arguments::ID)] = id;
    playlistObject[toString(Arguments::FAVOURITE)] = app_->playlist_manager()->IsPlaylistFavorite(id);

    QJsonArray songsArray;
    auto songs{app_->playlist_manager()->playlist(id)->GetAllSongs()};
    RemoteCurrentSong songInfo = RemoteCurrentSong(app_);
    for (const auto& song: songs){
        songsArray.append(songInfo.songData(song));
    }
    playlistObject[toString(RemoteTypes::Arguments::SONGS)] = songsArray;
    return playlistObject;
}

QJsonObject RemotePlaylist::receiveRemoteActive(const QStringList& args){
    if(args.size() != 1) return wrongNumArgs(1);

    bool ok;
    quint32 id{remoteconstants::parseUintArg(args, ok)};
    if(ok){
        Q_EMIT RemotePlaylist::setActivePlaylist(id);
        Q_EMIT RemotePlaylist::setCurrentPlaylistSignal(id);
        return RemoteJsonCreator::createResponse({
            field(MessageType::EVENT, toString(MessageType::EVENT)),
            field(Event::EVENT, toString(Event::SENT_ACTIVE_PLAYLIST)),
            field(Arguments::ID, static_cast<int>(id))
        });
    }
    return RemoteJsonCreator::createResponse({
        field(MessageType::ERROR, toString(MessageType::ERROR)),
        field(Error::ERROR, toString(Error::WRONG_ARGUMENT_SENT)),
        field(Arguments::ARGUMENT, args.first())
    });
}

QJsonObject RemotePlaylist::removeCurrentSongsPlaylist(const QStringList& args){
    if(args.size() != 2) return wrongNumArgs(2);
    qInfo()<< "removecurrentsongsplaylist called with: " << args;
    // QByteArray jsonData{songs}
    // QJsonDocument doc = QJsonDocument::fromJson(songs);


    //Need to send a playlist id and list of songs.
    auto songName{app_->playlist_manager()->current()->current_item_metadata().song_id()};
    Q_EMIT RemotePlaylist::removeCurrentSong();
    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::RESPONSE, toString(Response::REMOVED_SONG_FROM_PLAYLIST)),
        field(Arguments::NAME, songName)
    });
}

QJsonObject RemotePlaylist::removeDuplicatesPlaylist(const QStringList& args){
    if(args.size() != 1) return wrongNumArgs(1);

    bool ok;
    quint32 id{remoteconstants::parseUintArg(args, ok)};
    if(ok){
        int current{app_->playlist_manager()->current_id()};
        app_->playlist_manager()->SetCurrentPlaylist(id);
        Q_EMIT RemotePlaylist::removeDuplicates();
        app_->playlist_manager()->SetCurrentPlaylist(current);
    }

    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::REMOVED_DUPLICATES_FROM_PLAYLIST, toString(Response::REMOVED_DUPLICATES_FROM_PLAYLIST))
    });
}

QJsonObject RemotePlaylist::renameCurrentPlaylist(const QStringList& args){
    if(args.size() != 2) return wrongNumArgs(2);

    bool ok;
    quint32 id{remoteconstants::parseUintArg(args, ok)};
    QString name{args[1]};
    if(ok){
        Q_EMIT RemotePlaylist::remoteRenamePlaylist(id, name);
        return RemoteJsonCreator::createResponse({
          field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
          field(Response::RESPONSE, toString(Response::RENAME_PLAYLIST)),
          field(Arguments::NAME, app_->playlist_manager()->GetPlaylistName(id))
        });
    }
    return RemoteJsonCreator::createResponse({
        field(MessageType::ERROR, toString(MessageType::ERROR)),
        field(Error::ERROR, toString(Error::PLAYLIST_NOT_FOUND)),
        field(Arguments::NAME, args.first())
      });
}

QJsonObject RemotePlaylist::sendAllPlaylists() const {
    return makeAllPlaylists();
}

const PlaylistCmdMap& RemotePlaylist::sendCommandMap() const{
    return commandMap;
}

QJsonObject RemotePlaylist::sendCoverImage(){
    currentSong_->makeAlbumArt(app_->playlist_manager()->current()->current_item_metadata());

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

QJsonObject RemotePlaylist::sendRequestedPLaylist(const QStringList& args){
    if(args.size() != 1) return wrongNumArgs(1);

    bool ok;
    quint32 id{remoteconstants::parseUintArg(args, ok)};
    if(ok){
        return RemoteJsonCreator::createResponse({
            field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
            field(Response::RESPONSE, toString(Response::SEND_REQUESTED_PLAYLIST)),
            field(Arguments::PLAYLIST, makePlaylistData(id))
        });
    }
    return RemoteJsonCreator::createResponse({
        field(MessageType::ERROR, toString(MessageType::ERROR)),
        field(Error::ERROR, toString(Error::WRONG_ARGUMENT_SENT)),
        field(Arguments::ARGUMENT, args.first())
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

QJsonObject RemotePlaylist::setCurrentPlaylist(const QStringList& args){
    if(args.size() != 1) return wrongNumArgs(1);

    bool ok;
    quint32 id{remoteconstants::parseUintArg(args, ok)};
    if(ok){
        Q_EMIT RemotePlaylist::setCurrentPlaylistSignal(id);
        return RemoteJsonCreator::createResponse({
            field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
            field(Response::RESPONSE, toString(Response::SET_CURRENT_PLAYLIST_TO)),
            field(Arguments::NAME, app_->playlist_manager()->current()->objectName())
        });
    }
    return RemoteJsonCreator::createResponse({
        field(MessageType::ERROR, toString(MessageType::ERROR)),
        field(Error::ERROR, toString(Error::WRONG_ARGUMENT_SENT)),
        field(Arguments::ARGUMENT, args.first())
    });
}

QJsonObject RemotePlaylist::setFavouritePlaylist(const QStringList& args){
    if(args.size() != 2) return wrongNumArgs(2);

    bool ok;
    quint32 id{remoteconstants::parseUintArg(args, ok)};
    if(ok){
        bool isFavourite{static_cast<bool>(args[1].toUInt())};
        Q_EMIT RemotePlaylist::remoteFavouritePlaylist(id, isFavourite);
        return RemoteJsonCreator::createResponse({
            field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
            field(Response::RESPONSE, toString(Response::IS_PLAYLIST_A_FAVOURITE)),
            field(Arguments::IS_FAVOURITE, app_->playlist_manager()->playlist(id)->is_favorite())
        });
    }
    return RemoteJsonCreator::createResponse({
        field(MessageType::ERROR, toString(MessageType::ERROR)),
        field(Error::ERROR, toString(Error::PLAYLIST_NOT_FOUND))
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

QJsonObject RemotePlaylist::shuffleSinglePlaylist(const QStringList& args){
    if(args.size() != 1) return wrongNumArgs(1);

    bool ok;
    quint32 id{remoteconstants::parseUintArg(args,ok)};
    if(ok){
        int serverCurrent{app_->playlist_manager()->current_id()};
        app_->playlist_manager()->SetCurrentPlaylist(id);
        Q_EMIT RemotePlaylist::shufflePlaylist();
        app_->playlist_manager()->SetCurrentPlaylist(serverCurrent);

        return RemoteJsonCreator::createResponse({
            field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
            field(Response::SHUFFLED_PLAYLIST, toString(Response::SHUFFLED_PLAYLIST))
        });
    }
    return RemoteJsonCreator::createResponse({
        field(MessageType::ERROR, toString(MessageType::ERROR)),
        field(Error::ERROR, toString(Error::WRONG_ARGUMENT_SENT)),
        field(Arguments::ARGUMENT, args.first())
    });
}

QJsonObject RemotePlaylist::wrongNumArgs(const int num){
    return RemoteJsonCreator::createResponse({
        field(MessageType::ERROR, toString(MessageType::ERROR)),
        field(Error::ERROR, toString(Error::WRONG_NUMBER_ARGUMENTS_PASSED)),
        field(Arguments::REQUIRED, num)
    });
}

void RemotePlaylist::createCommandMap(){
    commandMap[u"clear-playlist"_s] = [this](const QStringList& args){ return clearRemoteCurrentPlaylist(args); };
    commandMap[u"close-playlist"_s] = [this](const QStringList& args){ return closeCurrentPlaylist(args); };
    commandMap[u"delete-playlist"_s] = [this](const QStringList& args){ return deleteCurrentDevicePlaylist(args); };
    commandMap[u"favourite-playlist"_s] = [this](const QStringList& args){ return setFavouritePlaylist(args); };
    commandMap[u"remove-duplicates-playlist"_s] = [this](const QStringList& args){ return removeDuplicatesPlaylist(args); };
    commandMap[u"remove-songs-playlist"_s] = [this](const QStringList& args){ return removeCurrentSongsPlaylist(args); };
    commandMap[u"rename-playlist"_s] = [this](const QStringList& args){ return renameCurrentPlaylist(args); };
    commandMap[u"send-active-playlist-songs"_s] = [this](const QStringList& args){ return receiveRemoteActive(args); };
    commandMap[u"send-all-playlists"_s] = [this](const auto&){ return makeAllPlaylists(); };
    commandMap[u"send-playlist"_s] = [this](const QStringList& args){ return sendRequestedPLaylist(args); };
    commandMap[u"request_cover"_s] = [this](const auto&){ return sendCoverImage(); };
    commandMap[u"set-current-playlist"_s] = [this](const QStringList& args){ return setCurrentPlaylist(args); };
    commandMap[u"shuffle-all-playlists"_s] = [this](const auto&){ return shuffleAllPlaylists(); };
    commandMap[u"shuffle-current-playlist"_s] = [this](const QStringList& args){ return shuffleSinglePlaylist(args); };
}