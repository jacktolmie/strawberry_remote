#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include "remoteplaylist.h"
#include "core/logging.h"
#include "playlist/playlistmanager.h"
#include "remotecontroller/remoteconstants.h"
RemotePlaylist::RemotePlaylist(Application* app, QObject *parent)
    : QObject{parent},
      app_(app)
{
  // Fill in the commandMap.
  RemotePlaylist::createCommandMap();
}

void RemotePlaylist::processCommand(QTcpSocket* clientSocket, const QString& command, const QStringList& args)
{
  qDebug() << "RemotePlaylist::processCommand called with command: "<< command << " and args: "<< args;

  if ( commandMap.contains(command)){
    qDebug()<< "RemotePlaylist::processCommand found: " << command;
    auto response{commandMap[command](args)};
    Q_EMIT RemotePlaylist::sendResponse(clientSocket,response );
  }
  else {
    QJsonObject response;
    response[QStringLiteral("response")] = QStringLiteral("Invalid command: %1").arg(command);
    Q_EMIT RemotePlaylist::sendResponse(clientSocket, response);
  }
}

QJsonObject RemotePlaylist::renamePlaylist(const QStringList& args)
{
  int id{app_->playlist_manager()->current_id()};
  QString name{args.first()};
  app_->playlist_manager()->Rename(id, name);
  return QJsonObject{{QStringLiteral("response"), QStringLiteral("Renamed playlist %1").arg(app_->playlist_manager()->GetPlaylistName(id))}};
}

QJsonObject RemotePlaylist::shuffleAllPlaylists()
{
  int currentId{app_->playlist_manager()->current_id()};

  QList<int> playlistIds{app_->playlist_manager()->playlist_ids()};

  for(auto& list: playlistIds){
    app_->playlist_manager()->SetCurrentPlaylist(list);
    app_->playlist_manager()->ShuffleCurrent();
  }
  app_->playlist_manager()->SetCurrentPlaylist(currentId);
  return QJsonObject{{QStringLiteral("response"), QStringLiteral("Shuffled all playlists")}};
}

QJsonObject RemotePlaylist::deleteCurrentPlaylist()
{
  QString name{app_->playlist_manager()->current()->objectName()};
  app_->playlist_manager()->Delete(app_->playlist_manager()->current_id());
  return QJsonObject{{QStringLiteral("response"), QStringLiteral("Deleted playlist %1").arg(name)}} ;
}

QJsonObject RemotePlaylist::favoritePlaylist()
{
  int playlistId{app_->playlist_manager()->current_id()};
  int isFavourite{app_->playlist_manager()->IsPlaylistFavorite(playlistId)};
  app_->playlist_manager()->Favorite(playlistId, !isFavourite);
  return QJsonObject{{QStringLiteral("response"), QStringLiteral("Is playlist a favourite? %1").arg(app_->playlist_manager()->playlist(playlistId)->is_favorite())}};
}

QJsonObject RemotePlaylist::setCurrentPlaylist(const int id)
{
  int currentPlaylist{app_->playlist_manager()->current_id()};
  if (currentPlaylist != id){
    app_->playlist_manager()->SetActivePlaylist(id);
    app_->playlist_manager()->SetCurrentOrOpen(id);
  }
  return QJsonObject{{QStringLiteral("response"), QStringLiteral("Set current playlist to %1").arg(app_->playlist_manager()->current()->objectName())}};
}

QJsonObject RemotePlaylist::makePlaylistData(const int id)
{
    QJsonObject playlistObject;
    playlistObject[QStringLiteral("name")] = app_->playlist_manager()->playlist_name(id);

    QJsonArray songsArray;
    auto songs{app_->playlist_manager()->playlist(id)->GetAllSongs()};

    for (const auto& song: songs){
      QJsonObject songData;
      songData[QStringLiteral("id")] =      song.id();
      songData[QStringLiteral("Artist")] =  song.artist();
      songData[QStringLiteral("Album")] =   song.album();
      songData[QStringLiteral("Title")] =   song.PrettyTitle();
      songsArray.append(songData);
    }

    playlistObject[QStringLiteral("songs")] = songsArray;

  return playlistObject;
}

QJsonObject RemotePlaylist::makeAllPlaylist()
{
  auto playlists{app_->playlist_manager()->GetAllPlaylists()};

  QJsonObject response;
  response[QStringLiteral("command")] = QStringLiteral("playlist_data");

  QJsonArray playlistArray;

  for (const auto& playlist: playlists){
    playlistArray.append(RemotePlaylist::makePlaylistData(playlist->id()));
  }

  response[QStringLiteral("playlistArray")] = playlistArray;

  return response;
}

QJsonObject RemotePlaylist::makeCurrentPlaylist()
{
  QJsonObject response;
  response[QStringLiteral("command")] = QStringLiteral("playlist_data");

  response[QStringLiteral("playlist")] = RemotePlaylist::makePlaylistData(app_->playlist_manager()->current_id());

  return response;
}

void RemotePlaylist::createCommandMap()
{
  // Lambda to check if args contains an int. Inside remoteconstants header.
  auto parseStringArg{remoteconstants::parseStringArg};
  auto parseUintArg{remoteconstants::parseUintArg};

  commandMap[QStringLiteral("clear-playlist")] = [this](const auto&){
    app_->playlist_manager()->ClearCurrent();
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Cleared playlist %1").arg(app_->playlist_manager()->current()->objectName())}};
  };
  commandMap[QStringLiteral("close-playlist")] = [this, parseUintArg](const QStringList& args){
    bool ok;
    quint32 id{parseUintArg(args, ok)};
    if (ok){
      bool closed{app_->playlist_manager()->Close(id)};
      if (closed) return QJsonObject{{QStringLiteral("response"), QStringLiteral("Playlist closed")}};
      else return QJsonObject{{QStringLiteral("response"), QStringLiteral("Playlist not closed")}};
    }
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Wrong argument sent: %1").arg(args.first())}};
  };
  commandMap[QStringLiteral("delete-current-playlist")] = [this](const auto&){ return RemotePlaylist::deleteCurrentPlaylist();};
  commandMap[QStringLiteral("favorite-playlist")] = [this](const auto&){ return RemotePlaylist::favoritePlaylist();};
  commandMap[QStringLiteral("make-all-playlists")] = [this](const auto&){ return RemotePlaylist::makeAllPlaylist();};
  commandMap[QStringLiteral("remove-current-song-playlist")] = [this](const auto&){
    auto songName{app_->playlist_manager()->current()->current_item_metadata().song_id()};
    app_->playlist_manager()->RemoveCurrentSong();
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Removed %1 from playlist").arg(songName)}};
  };
  commandMap[QStringLiteral("remove-duplicates-playlist")] = [this](const auto&){
    app_->playlist_manager()->RemoveDuplicatesCurrent();
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Removed duplicates from plalylist")}};
  };
  commandMap[QStringLiteral("rename-playlist")] = [this](const QStringList& args){return RemotePlaylist::renamePlaylist(args);};
  commandMap[QStringLiteral("set-current-playlist")] = [this, parseUintArg](const QStringList& args){
    bool ok;
    quint32 id{parseUintArg(args, ok)};
    if(ok){return RemotePlaylist::setCurrentPlaylist(id);}
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Wrong argument sent: ").arg(args.first())}};
  };
  commandMap[QStringLiteral("shuffle-playlist")] = [this](const auto&){
    app_->playlist_manager()->ShuffleCurrent();
    return QJsonObject{{QStringLiteral("response"), QStringLiteral("Shuffled playlist")}};
  };
  commandMap[QStringLiteral("shuffle-all-playlists")] = [this](const auto&){ return RemotePlaylist::shuffleAllPlaylists();};
  commandMap[QStringLiteral("send-playlist")] = [this](const auto&){ return RemotePlaylist::makeCurrentPlaylist();};
  commandMap[QStringLiteral("send-all-playlists")] = [this](const auto&){ return RemotePlaylist::makeAllPlaylist();};

  // commandMap[QStringLiteral("play")] = [this](const auto&){ app_->playlist_manager()->;};
  // commandMap[QStringLiteral("play")] = [this](const auto&){ app_->playlist_manager()->;};
}
