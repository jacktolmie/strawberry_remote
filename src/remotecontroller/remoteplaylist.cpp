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

void RemotePlaylist::createCommandMap()
{
  // Lambda to check if args contains an int. Inside remoteconstants header.
  auto parseStringArg{remoteconstants::parseStringArg};

  commandMap[QStringLiteral("shuffle-playlist")] = [this](const auto&){ app_->playlist_manager()->ShuffleCurrent();};
  commandMap[QStringLiteral("clear-playlist")] = [this](const auto&){ app_->playlist_manager()->ClearCurrent();};

  commandMap[QStringLiteral("load-playlist")] = [this, parseStringArg](const QStringList& args){
    bool ok;
    QString playlist = parseStringArg(args, ok);
    app_->playlist_manager()->Load(playlist);
  };

  commandMap[QStringLiteral("get-all-playlists")] = [this](const auto&){ app_->playlist_manager()->GetAllPlaylists();};
  commandMap[QStringLiteral("rename-playlist")] = [this](const QStringList& args){RemotePlaylist::renamePlaylist(args);};
  commandMap[QStringLiteral("shuffle-all-playlists")] = [this](const auto&){ RemotePlaylist::shuffleAllPlaylists();};


  // commandMap[QStringLiteral("play")] = [this](const auto&){ app_->playlist_manager()->;};


}

/*
    QStringLiteral("shuffle-playlist"),
    QStringLiteral("clear-playlist"),
    QStringLiteral("delete-playlist"),
    QStringLiteral("get-all-playlists"), returns a QList<Playlist>
    QStringLiteral("save-playlist"),
    QStringLiteral("smart-playlist"),
    QStringLiteral("rename-playlist"),
    QStringLiteral("delete-playlist")

  if (command == QStringLiteral("get-all-playlists")){
    auto list = app_->playlist_manager()->GetAllPlaylists();
    for(auto& l: list){
      auto test = l->GetAllSongs();
      for(auto& song: test) qDebug() <<"Remote song: " << song.PrettyTitle();
    }
  }
*/
