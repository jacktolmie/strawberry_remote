#ifndef REMOTEPLAYLIST_H
#define REMOTEPLAYLIST_H

#include <QObject>
#include <QDebug>

#include "core/application.h"

class QTcpSocket;

class RemotePlaylist : public QObject
{
     Q_OBJECT

  Application *app_;

  void createCommandMap();
  QMap<QString, std::function<QJsonObject(const QStringList&)>> commandMap;

  // Rename current playlist. Send command rename-playlist <new name>.
  QJsonObject renamePlaylist(const QStringList& args);

  // Shuffle all playlists.
  QJsonObject shuffleAllPlaylists();

  // Delete current playlist.
  QJsonObject deleteCurrentPlaylist();

  // Make playlist a favourite or not.
  QJsonObject favoritePlaylist();

  // Set the playlist as current. Send set-current-playlist <playlist ID>.
  QJsonObject setCurrentPlaylist(const int id);

  // Make playlists to send back to device.
  QJsonObject makeAllPlaylist();
  QJsonObject makeCurrentPlaylist();
  QJsonObject makePlaylistData(const int id);

Q_SIGNALS:
  // Send playlist changes etc.
  void sendResponse(QTcpSocket* clientSocket, QJsonObject& response);

public:
    explicit RemotePlaylist(Application *app, QObject *parent = nullptr);
    ~RemotePlaylist() = default;

    void processCommand(QTcpSocket* clientSocket, const QString& command, const QStringList& args);
};

#endif // REMOTEPLAYLIST_H
