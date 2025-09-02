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

void RemotePlaylist::processCommand(const QString& command, const QStringList& args)
{
  qDebug() << "RemotePlaylist::processCommand called with command: "<< command << " and args: "<< args;

  if ( commandMap.contains(command)){
    qDebug()<< "RemotePlaylist::processCommand found: " << command;
    commandMap[command](args);
  }

}

void RemotePlaylist::renamePlaylist(const QStringList& args)
{
  int id{app_->playlist_manager()->current_id()};
  QString name{args.first()};
  app_->playlist_manager()->Rename(id, name);
}

void RemotePlaylist::shuffleAllPlaylists()
{
  int currentId{app_->playlist_manager()->current_id()};

  QList<int> playlistIds{app_->playlist_manager()->playlist_ids()};

  for(auto& list: playlistIds){
    app_->playlist_manager()->SetCurrentPlaylist(list);
    app_->playlist_manager()->ShuffleCurrent();
  }
  app_->playlist_manager()->SetCurrentPlaylist(currentId);
}

void RemotePlaylist::deleteCurrentPlaylist()
{
  app_->playlist_manager()->Delete(app_->playlist_manager()->current_id());
}

void RemotePlaylist::favoritePlaylist()
{
  int playlistId{app_->playlist_manager()->current_id()};
  int isFavourite{app_->playlist_manager()->IsPlaylistFavorite(playlistId)};
  app_->playlist_manager()->Favorite(playlistId, !isFavourite);
}

void RemotePlaylist::setCurrentPlaylist(const int id)
{
  int currentPlaylist{app_->playlist_manager()->current_id()};
  if (currentPlaylist != id){
    app_->playlist_manager()->SetActivePlaylist(id);
    app_->playlist_manager()->SetCurrentOrOpen(id);
  }
}

void RemotePlaylist::makeAllPlaylist()
{
  auto playlists{app_->playlist_manager()->GetAllPlaylists()};

  QJsonObject response;
  response[QStringLiteral("command")] = QStringLiteral("playlist_data");

  QJsonArray playlistArray;

  for (const auto& playlist: playlists){
    QJsonObject playlistObject;
    playlistObject[QStringLiteral("name")] = playlist->objectName();

    QJsonArray songsArray;
    auto songs{playlist->GetAllSongs()};

    for (const auto& song: songs){
      songsArray.append(song.PrettyTitle());
    }

    playlistObject[QStringLiteral("songs")] = songsArray;

    playlistArray.append(playlistObject);
  }

  response[QStringLiteral("playlists")] = playlistArray;

  Q_EMIT RemotePlaylist::sendAllPlaylists(response);
}

void RemotePlaylist::makeCurrentPlaylist()
{
  int id{app_->playlist_manager()->current_id()};
  auto playlist{app_->playlist_manager()->playlist(id)};

  QJsonObject response;
  response[QStringLiteral("command")] = QStringLiteral("playlist_data");

  QJsonObject playlistObject;
  playlistObject[QStringLiteral("name")] = playlist->objectName();

  QJsonArray songsArray;
  auto songs{playlist->GetAllSongs()};

  for (const auto& song: songs){
    songsArray.append(song.PrettyTitle());
  }

  playlistObject[QStringLiteral("songs")] = songsArray;

  response[QStringLiteral("songs")] = playlistObject;

  Q_EMIT RemotePlaylist::sendCurrentPlaylist(response);
}

void RemotePlaylist::createCommandMap()
{
  // Lambda to check if args contains an int. Inside remoteconstants header.
  auto parseStringArg{remoteconstants::parseStringArg};
  auto parseUintArg{remoteconstants::parseUintArg};

  commandMap[QStringLiteral("clear-playlist")] = [this](const auto&){ app_->playlist_manager()->ClearCurrent();};
  commandMap[QStringLiteral("close-playlist")] = [this, parseUintArg](const QStringList& args){
    bool ok;
    quint32 id{parseUintArg(args, ok)};
    if (ok) qDebug() << app_->playlist_manager()->Close(id);
  };
  commandMap[QStringLiteral("delete-current-playlist")] = [this](const auto&){ RemotePlaylist::deleteCurrentPlaylist();};
  commandMap[QStringLiteral("favorite-playlist")] = [this](const auto&){ RemotePlaylist::favoritePlaylist();};
  commandMap[QStringLiteral("get-all-playlists")] = [this](const auto&){ app_->playlist_manager()->GetAllPlaylists();};
  commandMap[QStringLiteral("remove-current-song-playlist")] = [this](const auto&){ app_->playlist_manager()->RemoveCurrentSong();};
  commandMap[QStringLiteral("remove-duplicates-playlist")] = [this](const auto&){ app_->playlist_manager()->RemoveDuplicatesCurrent();};
  commandMap[QStringLiteral("rename-playlist")] = [this](const QStringList& args){RemotePlaylist::renamePlaylist(args);};
  commandMap[QStringLiteral("set-current-playlist")] = [this, parseUintArg](const QStringList& args){
    bool ok;
    quint32 id{parseUintArg(args, ok)};
    if(ok)RemotePlaylist::setCurrentPlaylist(id);
  };
  commandMap[QStringLiteral("shuffle-playlist")] = [this](const auto&){ app_->playlist_manager()->ShuffleCurrent();};
  commandMap[QStringLiteral("shuffle-all-playlists")] = [this](const auto&){ RemotePlaylist::shuffleAllPlaylists();};
  commandMap[QStringLiteral("send-playlist")] = [this](const auto&){ RemotePlaylist::shuffleAllPlaylists();};
  commandMap[QStringLiteral("send-all-playlists")] = [this](const auto&){ RemotePlaylist::shuffleAllPlaylists();};

  // commandMap[QStringLiteral("play")] = [this](const auto&){ app_->playlist_manager()->;};
  // commandMap[QStringLiteral("play")] = [this](const auto&){ app_->playlist_manager()->;};
}


  // auto all = app_->playlist_manager()->playlist_ids();
  // for(auto ids: std::as_const(all)) qDebug() <<"Remote Playlist id: "<< ids;
