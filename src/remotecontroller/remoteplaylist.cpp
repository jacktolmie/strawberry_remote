#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include "remoteplaylist.h"
#include "playlist/playlist.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistbackend.h"
#include "core/player.h"
#include "remotecontroller/remoteconstants.h"
#include "remotecontroller/remotecurrentsong.h"

RemotePlaylist::RemotePlaylist(Application* app, QObject *parent)
    : QObject{parent},
      app_(app)
{
  // Fill in the commandMap.
  RemotePlaylist::createCommandMap();

  QObject::connect(this, &RemotePlaylist::clearPlaylist , &*app_->playlist_manager(), &PlaylistManager::ClearCurrent);
  QObject::connect(this, &RemotePlaylist::closePlaylist , &*app_->playlist_manager(), &PlaylistManager::Close);
  QObject::connect(this, &RemotePlaylist::deletePlaylist , &*app_->playlist_manager(), &PlaylistManager::Delete);
  QObject::connect(this, &RemotePlaylist::remoteFavouritePlaylist , &*app_->playlist_manager(), &PlaylistManager::Favorite);
  QObject::connect(this, &RemotePlaylist::removeCurrentSong , &*app_->playlist_manager(), &PlaylistManager::RemoveCurrentSong);
  QObject::connect(this, &RemotePlaylist::removeDuplicates , &*app_->playlist_manager(), &PlaylistManager::RemoveDuplicatesCurrent);
  QObject::connect(this, &RemotePlaylist::remoteRenamePlaylist , &*app_->playlist_manager(), &PlaylistManager::Rename);
  QObject::connect(this, &RemotePlaylist::setCurrentPlaylistSignal , &*app_->playlist_manager(), &PlaylistManager::SetCurrentPlaylist);
  QObject::connect(this, &RemotePlaylist::shufflePlaylist , &*app_->playlist_manager(), &PlaylistManager::ShuffleCurrent);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::playlistChanged, this, &RemotePlaylist::playlistChanged);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::PlaylistDeleted, this, &RemotePlaylist::deletePlaylist);
  QObject::connect(this, &RemotePlaylist::deletePlaylist, this, &RemotePlaylist::deleteServerPlaylist);

  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::PlaylistClosed, this, &RemotePlaylist::closePlaylist);
  QObject::connect(this, &RemotePlaylist::closePlaylist, this, &RemotePlaylist::closeServerPlaylist);

  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::PlaylistFavorited, this, &RemotePlaylist::serverFavouritePlaylist);
  QObject::connect(this, &RemotePlaylist::serverFavouritePlaylist, this, &RemotePlaylist::favouriteServerPlaylist);

  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::renamePlaylist, this, &RemotePlaylist::serverRenamePlaylist);
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::sendActivePlaylist, this, &RemotePlaylist::activeChanged);
  QObject::connect(this, &RemotePlaylist::setActivePlaylist, &*app_->playlist_manager(), &PlaylistManager::SetActivePlaylist);

// QObject::connect(this, &RemotePlaylist::clearPlaylist , &*app_->playlist_manager(), &PlaylistManager::);
}

QJsonObject RemotePlaylist::renameCurrentPlaylist(const QStringList& args)
{
  if(args.size() < 2)   return QJsonObject{{QStringLiteral("response"), QStringLiteral("Not enough arguments passed. Needs 2")}};

  bool ok;
  quint32 id{remoteconstants::parseUintArg(args, ok)};
  QString name{args[1]};
  if(ok){
    Q_EMIT RemotePlaylist::remoteRenamePlaylist(id, name);
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Renamed playlist %1").arg(app_->playlist_manager()->GetPlaylistName(id))}};
  }
  return QJsonObject{{QStringLiteral("response"), QStringLiteral("Playlist %1 not found").arg(args[0])}};
}

QJsonObject RemotePlaylist::shuffleAllPlaylists()
{
  int currentId{app_->playlist_manager()->current_id()};

  QList<int> playlistIds{app_->playlist_manager()->playlist_ids()};

  for(auto& list: playlistIds){
    Q_EMIT RemotePlaylist::setCurrentPlaylistSignal(list);
    Q_EMIT RemotePlaylist::shufflePlaylist();
  }
  Q_EMIT RemotePlaylist::setCurrentPlaylistSignal(currentId);
  return QJsonObject{{QStringLiteral("response"), QStringLiteral("Shuffled all playlists")}};
}

QJsonObject RemotePlaylist::deleteCurrentDevicePlaylist(const QStringList& args)
{
  if(args.empty()) return QJsonObject{{QStringLiteral("response"), QStringLiteral("Not enough arguments passed. Needs 1")}};

  QString name{QStringLiteral("No name")};
  bool ok;
  quint32 id{remoteconstants::parseUintArg(args,ok)};
  if(ok){
      name = {app_->playlist_backend()->GetPlaylist(id).name };
      Q_EMIT RemotePlaylist::deletePlaylist(id);
      return QJsonObject{{QStringLiteral("response"), QStringLiteral("Deleted playlist %1").arg(name)}} ;
  }
  return QJsonObject{{QStringLiteral("response"), QStringLiteral("No playlist %1 found").arg(name)}} ;
}

QJsonObject RemotePlaylist::setFavouritePlaylist(const QStringList& args)
{
  if(args.size() < 2) return QJsonObject{{QStringLiteral("response"), QStringLiteral("Not enough arguments passed. Needs 2")}};

  bool ok;
  quint32 id{remoteconstants::parseUintArg(args,ok)};

  if(ok){
    bool isFavourite{static_cast<bool>(args[1].toUInt())};   //{app_->playlist_manager()->IsPlaylistFavorite(id)};
    Q_EMIT RemotePlaylist::remoteFavouritePlaylist(id, isFavourite);
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Is playlist a favourite? %1").arg(app_->playlist_manager()->playlist(id)->is_favorite())}};
  }
  return QJsonObject{{QStringLiteral("response"), QStringLiteral("Playlist not found")}} ;
}

QJsonObject RemotePlaylist::setCurrentPlaylist(const QStringList& args)
{
  if(args.empty()) return QJsonObject{{QStringLiteral("response"), QStringLiteral("Not enough arguments passed. Needs 1")}};

  bool ok;
  quint32 id{remoteconstants::parseUintArg(args, ok)};
  if(ok){
    Q_EMIT RemotePlaylist::setCurrentPlaylistSignal(id);
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Set current playlist to %1").arg(app_->playlist_manager()->current()->objectName())}};
  }
  return QJsonObject{{QStringLiteral("response"), QStringLiteral("Wrong argument sent: ").arg(args.first())}};
}

QJsonObject RemotePlaylist::makePlaylistData(const int id)
{
  QJsonObject playlistObject;
  playlistObject[QStringLiteral("name")] = app_->playlist_manager()->playlist_name(id);
  playlistObject[QStringLiteral("id")] = id;
  // Array to hold the songs in the playlist.
  QJsonArray songsArray;

  // Get all songs in the sent playlist id.
  auto songs{app_->playlist_manager()->playlist(id)->GetAllSongs()};

  // Make object to call up the creation of the data for each song in playlist.
  RemoteCurrentSong songInfo = RemoteCurrentSong(app_);

  // Iterate through all songs in the playlist, and add data to returned JsonObject.
  for (const auto& song: songs){
    songsArray.append(songInfo.songInfo(song));
  }

  playlistObject[QStringLiteral("songs")] = songsArray;

  return playlistObject;
}

QJsonObject RemotePlaylist::makeAllPlaylist()
{
  QJsonArray playlistArray;

  auto playlists{app_->playlist_manager()->GetAllPlaylists()};

  for (const auto& playlist: playlists){
    QJsonObject singlePlaylistData = RemotePlaylist::makePlaylistData(playlist->id());
    playlistArray.append(singlePlaylistData);
  }

  QJsonObject response;
  response[QStringLiteral("command")] = QStringLiteral("all_playlists");
  response[QStringLiteral("playlists")] = playlistArray;
  return response;
}

QJsonObject RemotePlaylist::makeCurrentPlaylist()
{
  QJsonObject response;
  response[QStringLiteral("command")] = QStringLiteral("current_playlist");

  response[QStringLiteral("playlist")] = RemotePlaylist::makePlaylistData(app_->playlist_manager()->current_id());

  return response;
}

PlaylistCmdMap& RemotePlaylist::sendCommandMap()
{
  return commandMap;
}

QJsonObject RemotePlaylist::closeCurrentPlaylist(const QStringList& args)
{
  bool ok;
  quint32 id{remoteconstants::parseUintArg(args, ok)};
  if (ok){
    int current{app_->playlist_manager()->current_id()};
    Q_EMIT closePlaylist(id);
    if (current != app_->playlist_manager()->current_id()) return QJsonObject{{QStringLiteral("response"), QStringLiteral("Playlist closed")}};
    else return QJsonObject{{QStringLiteral("response"), QStringLiteral("Playlist not closed")}};
  }
  return QJsonObject{{QStringLiteral("response"), QStringLiteral("Wrong argument sent: %1").arg(args.first())}};
}

void RemotePlaylist::playlistChanged()
{
  auto playlist{RemotePlaylist::makeCurrentPlaylist()};
  Q_EMIT RemotePlaylist::sendResponse(playlist);
}

void RemotePlaylist::deleteServerPlaylist(const int id)
{
  QJsonObject response{{QStringLiteral("response"), QStringLiteral("Deleted playlist with ID: %1").arg(id)}};
  Q_EMIT RemotePlaylist::sendResponse(response);
}

void RemotePlaylist::closeServerPlaylist(const int id)
{
  QJsonObject response{{QStringLiteral("response"), QStringLiteral("Close playlist with ID: %1").arg(id)}};
  Q_EMIT RemotePlaylist::sendResponse(response);
}

void RemotePlaylist::favouriteServerPlaylist(const int id, bool favourite)
{
  QJsonObject response;
  response[QStringLiteral("command")] = QStringLiteral("favourite_playlist");
  response[QStringLiteral("id")] = id;
  response[QStringLiteral("favourite")] = favourite;
  Q_EMIT RemotePlaylist::sendResponse(response);
}

void RemotePlaylist::serverRenamePlaylist(const int id, const QString& name)
{
  QJsonObject response;
  response[QStringLiteral("command")] = QStringLiteral("rename_playlist");
  response[QStringLiteral("id")] = id;
  response[QStringLiteral("name")] = name;
  Q_EMIT RemotePlaylist::sendResponse(response);
}

void RemotePlaylist::activeChanged(const int id)
{
  QJsonObject response;
  response[QStringLiteral("command")] = QStringLiteral("active_playlist");
  response[QStringLiteral("id")] = id;
  Q_EMIT RemotePlaylist::sendResponse(response);
}

QJsonObject RemotePlaylist::sendRemoteActive(const QStringList& args)
{
  if(args.empty()) return QJsonObject{{QStringLiteral("response"), QStringLiteral("Not enough arguments passed. Needs 1")}};

  bool ok;
  quint32 id{remoteconstants::parseUintArg(args, ok)};
  if(ok){
    Q_EMIT RemotePlaylist::setActivePlaylist(id);
    Q_EMIT RemotePlaylist::setCurrentPlaylistSignal(id);
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Sent active playlist")}};
  }
  return QJsonObject{{QStringLiteral("response"), QStringLiteral("Wrong argument sent")}};
}

void RemotePlaylist::createCommandMap()
{
  commandMap[QStringLiteral("clear-playlist")] = [this](const auto&){
    Q_EMIT RemotePlaylist::clearPlaylist();
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Cleared playlist %1").arg(app_->playlist_manager()->current()->objectName())}};
  };
  commandMap[QStringLiteral("close-playlist")] = [this](const QStringList& args){ return RemotePlaylist::closeCurrentPlaylist(args);};
  commandMap[QStringLiteral("delete-current-playlist")] = [this](const QStringList& args){return RemotePlaylist::deleteCurrentDevicePlaylist(args);};
  commandMap[QStringLiteral("favourite-playlist")] = [this](const QStringList& args){ return RemotePlaylist::setFavouritePlaylist(args);};
  commandMap[QStringLiteral("remove-current-song-playlist")] = [this](const auto&){
    auto songName{app_->playlist_manager()->current()->current_item_metadata().song_id()};
    Q_EMIT RemotePlaylist::removeCurrentSong();
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Removed %1 from playlist").arg(songName)}};
  };
  commandMap[QStringLiteral("remove-duplicates-playlist")] = [this](const auto&){
    Q_EMIT RemotePlaylist::removeDuplicates();
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Removed duplicates from plalylist")}};
  };
  commandMap[QStringLiteral("rename-playlist")] = [this](const QStringList& args){return RemotePlaylist::renameCurrentPlaylist(args);};
  commandMap[QStringLiteral("set-current-playlist")] = [this](const QStringList& args){ return RemotePlaylist::setCurrentPlaylist(args);};
  commandMap[QStringLiteral("shuffle-playlist")] = [this](const auto&){
    Q_EMIT RemotePlaylist::shufflePlaylist();
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Shuffled playlist")}};
  };
  commandMap[QStringLiteral("shuffle-all-playlists")] = [this](const auto&){ return RemotePlaylist::shuffleAllPlaylists();};
  commandMap[QStringLiteral("send-active-playlist")] = [this](const QStringList& args){ return RemotePlaylist::sendRemoteActive(args);};
  commandMap[QStringLiteral("send-playlist")] = [this](const auto&){ return RemotePlaylist::makeCurrentPlaylist();};
  commandMap[QStringLiteral("send-all-playlists")] = [this](const auto&){ return RemotePlaylist::makeAllPlaylist();};

  // commandMap[QStringLiteral("play")] = [this](const auto&){ app_->playlist_manager()->;};
  // commandMap[QStringLiteral("play")] = [this](const auto&){ app_->playlist_manager()->;};
}
