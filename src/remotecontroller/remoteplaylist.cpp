#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include "remoteplaylist.h"
#include "playlist/playlistmanager.h"
#include "playlist/playlistbackend.h"
#include "core/player.h"
#include "remotecontroller/remoteconstants.h"
#include "remotecontroller/remotecurrentsong.h"
#include "remotecontroller/remotejsoncreator.h"

using namespace Qt::Literals::StringLiterals;

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
  if(args.size() < 2)   return RemoteJsonCreator::createResponse(u"error"_s, u"Not enough arguments passed. Needs 2"_s);

  bool ok;
  quint32 id{remoteconstants::parseUintArg(args, ok)};
  QString name{args[1]};
  if(ok){
    Q_EMIT RemotePlaylist::remoteRenamePlaylist(id, name);

    return RemoteJsonCreator::createResponse({
      {u"response"_s, u"Renames playlist"_s},
      {u"value"_s, app_->playlist_manager()->GetPlaylistName(id)}
    });
  }

  return RemoteJsonCreator::createResponse({ {u"error"_s, u"Playlist not found"_s}, {u"value"_s, args[0]} });
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
  return RemoteJsonCreator::createResponse(u"response"_s, u"Shuffled all playlists"_s);
}

QJsonObject RemotePlaylist::deleteCurrentDevicePlaylist(const QStringList& args)
{
  if(args.empty()) return RemoteJsonCreator::createResponse(u"error"_s, u"Not enough arguments passed. Needs 1"_s);

  QString name{u"No name"_s};
  bool ok;
  quint32 id{remoteconstants::parseUintArg(args,ok)};
  if(ok){
      name = {app_->playlist_backend()->GetPlaylist(id).name };
      Q_EMIT RemotePlaylist::deletePlaylist(id);

      return RemoteJsonCreator::createResponse({ {u"response"_s, u"Deleted playlist"_s}, {u"value"_s, name} });
  }
  return RemoteJsonCreator::createResponse({ {u"error"_s, u"Playlist not found"_s}, {u"value"_s, name} });
}

QJsonObject RemotePlaylist::setFavouritePlaylist(const QStringList& args)
{
  if(args.size() < 2) return RemoteJsonCreator::createResponse(u"error"_s, u"Not enough arguments passed. Needs 2"_s);

  bool ok;
  quint32 id{remoteconstants::parseUintArg(args,ok)};

  if(ok){
    bool isFavourite{static_cast<bool>(args[1].toUInt())};
    Q_EMIT RemotePlaylist::remoteFavouritePlaylist(id, isFavourite);
    return RemoteJsonCreator::createResponse({
      {u"response"_s, u"Is playlist a facourite"_s},
      {u"value"_s, QString::number(app_->playlist_manager()->playlist(id)->is_favorite())}
    });
  }
  return RemoteJsonCreator::createResponse(u"error"_s, u"Playlist not found"_s);
}

QJsonObject RemotePlaylist::setCurrentPlaylist(const QStringList& args)
{
  if(args.empty()) return RemoteJsonCreator::createResponse(u"error"_s, u"Not enough arguments passed. Needs 1"_s);

  bool ok;
  quint32 id{remoteconstants::parseUintArg(args, ok)};
  if(ok){
    Q_EMIT RemotePlaylist::setCurrentPlaylistSignal(id);
    return RemoteJsonCreator::createResponse({
      {u"response"_s, u"Set current playlist to"_s},
      {u"value"_s, app_->playlist_manager()->current()->objectName()}
    });
  }
  return RemoteJsonCreator::createResponse({ {u"error"_s, u"Wrong argument sent"_s}, {u"value"_s, args.first()} });
}

QJsonObject RemotePlaylist::makePlaylistData(const int id)
{
  QJsonObject playlistObject;
  playlistObject[u"name"_s] = app_->playlist_manager()->playlist_name(id);
  playlistObject[u"id"_s] = id;
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

  playlistObject[u"songs"_s] = songsArray;

  return playlistObject;
}

QJsonObject RemotePlaylist::makeAllPlaylists()
{
  QJsonArray playlistArray;

  auto playlists{app_->playlist_manager()->GetAllPlaylists()};

  for (const auto& playlist: playlists){
    QJsonObject singlePlaylistData = RemotePlaylist::makePlaylistData(playlist->id());
    playlistArray.append(singlePlaylistData);
  }

  QJsonObject response;
  response[u"type"_s] = u"event"_s;
  response[u"event"_s] = u"make_all_playlists"_s;
  response[u"playlists"_s] = playlistArray;
  return response;
}

QJsonObject RemotePlaylist::makeCurrentPlaylist()
{
  QJsonObject response;
  response[u"type"_s] = u"event"_s;
  response[u"event"_s] = u"make_current_playlist"_s;

  response[u"playlist"_s] = RemotePlaylist::makePlaylistData(app_->playlist_manager()->current_id());

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
    if (current != app_->playlist_manager()->current_id()) return RemoteJsonCreator::createResponse(u"response"_s, u"Playlist closed"_s);
    else return RemoteJsonCreator::createResponse(u"error"_s, u"Playlist not closed"_s);
  }
  return RemoteJsonCreator::createResponse({ {u"error"_s, u"Wrong argument sent"_s}, {u"value"_s, args.first()} });
}

void RemotePlaylist::playlistChanged()
{
  Q_EMIT RemotePlaylist::sendResponse(RemotePlaylist::makeCurrentPlaylist());
  // auto playlist{RemotePlaylist::makeCurrentPlaylist()};
  // Q_EMIT RemotePlaylist::sendResponse(playlist);
}

void RemotePlaylist::deleteServerPlaylist(const int id)
{
  Q_EMIT RemotePlaylist::sendResponse(RemoteJsonCreator::createResponse({
    {u"response"_s, u"Deleted playlist with ID"_s},
    {u"value"_s, QString::number(id)}
  }));
}

void RemotePlaylist::closeServerPlaylist(const int id)
{
  Q_EMIT RemotePlaylist::sendResponse(RemoteJsonCreator::createResponse({
    {u"response"_s, u"Closed playlist with ID"_s},
    {u"value"_s, QString::number(id)}
  }));
}

void RemotePlaylist::favouriteServerPlaylist(const int id, bool favourite)
{
  Q_EMIT RemotePlaylist::sendResponse(RemoteJsonCreator::createResponse({
    {u"event"_s, u"favourite_playlist"_s},
    {u"id"_s, QString::number(id)},
    {u"favourite"_s, QString::number(favourite)}
  }));
}

void RemotePlaylist::serverRenamePlaylist(const int id, const QString& name)
{
  Q_EMIT RemotePlaylist::sendResponse(RemoteJsonCreator::createResponse({
    {u"event"_s, u"rename_playlist"_s},
    {u"id"_s, QString::number(id)},
    {u"name"_s, name}
  }));
}

void RemotePlaylist::activeChanged(const int id)
{
  Q_EMIT RemotePlaylist::sendResponse(RemoteJsonCreator::createResponse({
    {u"event"_s, u"active_playlist"_s},
    {u"id"_s, QString::number(id)}
  }));
}

QJsonObject RemotePlaylist::sendRemoteActive(const QStringList& args)
{
  if(args.empty()) return RemoteJsonCreator::createResponse(u"error"_s, u"Not enough arguments passed. Needs 1"_s);

  bool ok;
  quint32 id{remoteconstants::parseUintArg(args, ok)};
  if(ok){
    Q_EMIT RemotePlaylist::setActivePlaylist(id);
    Q_EMIT RemotePlaylist::setCurrentPlaylistSignal(id);

    return RemoteJsonCreator::createResponse(u"response"_s, u"Sent active playlist"_s);
  }
  return RemoteJsonCreator::createResponse(u"error"_s, u"Wrong argument sent"_s);
}

void RemotePlaylist::createCommandMap()
{
  commandMap[u"clear-playlist"_s] = [this](const auto&){
    Q_EMIT RemotePlaylist::clearPlaylist();
    return RemoteJsonCreator::createResponse({ {u"response"_s, u"Cleared playlist"_s}, {u"value"_s, app_->playlist_manager()->current()->objectName()} });
  };
  commandMap[u"close-playlist"_s] = [this](const QStringList& args){ return RemotePlaylist::closeCurrentPlaylist(args);};
  commandMap[u"delete-current-playlist"_s] = [this](const QStringList& args){return RemotePlaylist::deleteCurrentDevicePlaylist(args);};
  commandMap[u"favourite-playlist"_s] = [this](const QStringList& args){ return RemotePlaylist::setFavouritePlaylist(args);};
  commandMap[u"remove-current-song-playlist"_s] = [this](const auto&){
    auto songName{app_->playlist_manager()->current()->current_item_metadata().song_id()};
    Q_EMIT RemotePlaylist::removeCurrentSong();
    return RemoteJsonCreator::createResponse({ {u"response"_s, u"Removed song from playlist"_s}, {u"value"_s, songName} });
  };
  commandMap[u"remove-duplicates-playlist"_s] = [this](const auto&){
    Q_EMIT RemotePlaylist::removeDuplicates();
    return RemoteJsonCreator::createResponse(u"response"_s, u"Removed duplicates from plalylist"_s);
  };
  commandMap[u"rename-playlist"_s] = [this](const QStringList& args){return RemotePlaylist::renameCurrentPlaylist(args);};
  commandMap[u"set-current-playlist"_s] = [this](const QStringList& args){ return RemotePlaylist::setCurrentPlaylist(args);};
  commandMap[u"shuffle-playlist"_s] = [this](const auto&){
    Q_EMIT RemotePlaylist::shufflePlaylist();
    return RemoteJsonCreator::createResponse(u"response"_s, u"Shuffled playlist"_s);
  };
  commandMap[u"shuffle-all-playlists"_s] = [this](const auto&){ return RemotePlaylist::shuffleAllPlaylists();};
  commandMap[u"send-active-playlist"_s] = [this](const QStringList& args){ return RemotePlaylist::sendRemoteActive(args);};
  commandMap[u"send-playlist"_s] = [this](const auto&){ return RemotePlaylist::makeCurrentPlaylist();};
  commandMap[u"send-all-playlists"_s] = [this](const auto&){ return RemotePlaylist::makeAllPlaylists();};

  // commandMap[u"play"_s] = [this](const auto&){ app_->playlist_manager()->;};
  // commandMap[u"play"_s] = [this](const auto&){ app_->playlist_manager()->;};
}
