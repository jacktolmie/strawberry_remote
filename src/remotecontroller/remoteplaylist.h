#ifndef REMOTEPLAYLIST_H
#define REMOTEPLAYLIST_H

#include <QObject>
#include <QDebug>

#include "core/application.h"

class QTcpSocket;

using PlaylistCmdMap = QMap<QString, std::function<QJsonObject(const QStringList&)>>;

class RemotePlaylist : public QObject
{
  Q_OBJECT

  Application *app_;

  PlaylistCmdMap commandMap;
  void createCommandMap();

  QJsonObject closeCurrentPlaylist(const QStringList& args);
  // Rename current playlist. Send command rename-playlist <new name>.
  QJsonObject renameCurrentPlaylist(const QStringList& args);

  // Shuffle all playlists.
  QJsonObject shuffleAllPlaylists();

  // Delete current playlist.
  QJsonObject deleteCurrentDevicePlaylist(const QStringList& args);

  // Make playlist a favourite or not.
  QJsonObject setFavouritePlaylist(const QStringList& args);

  // Set the playlist as current. Send set-current-playlist <playlist ID>.
  QJsonObject setCurrentPlaylist(const QStringList& args);

  // Make playlists to send back to device.
  QJsonObject makeAllPlaylist();
  QJsonObject makeCurrentPlaylist();
  QJsonObject makePlaylistData(const int id);

Q_SIGNALS:
  void sendResponse(QJsonObject& response);
  void clearPlaylist();
  void closePlaylist(const int id);
  void deletePlaylist(const int id);
  void remoteFavouritePlaylist(const int id, bool isFavourite);
  void serverFavouritePlaylist(const int id, bool isFavourite);
  void removeCurrentSong();
  void removeDuplicates();
  void remoteRenamePlaylist(const int id, const QString& name);
  void setCurrentPlaylistSignal(const int id);
  void shufflePlaylist();
  // void shuffleAllPlaylists();
  // Send playlist changes etc.
  // void sendResponse(QTcpSocket* clientSocket, QJsonObject& response);

private Q_SLOTS:

  // If playlist is changed on server, send updated playlist.
  void closeServerPlaylist(const int id);
  void deleteServerPlaylist(const int id);
  void favouriteServerPlaylist(const int id, bool favourite);
  void playlistChanged();
  void serverRenamePlaylist(const int id, const QString& name);

public:
  explicit RemotePlaylist(Application *app, QObject *parent = nullptr);
  ~RemotePlaylist() = default;

  PlaylistCmdMap& sendCommandMap();
};

#endif // REMOTEPLAYLIST_H
