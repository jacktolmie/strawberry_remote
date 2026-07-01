#ifndef REMOTEPLAYLIST_H
#define REMOTEPLAYLIST_H

#include <QObject>
#include <QDebug>
#include <QTimer>

#include "core/application.h"
#include "remotecontroller/remotecurrentsong.h"

using PlaylistCmdMap = QMap<QString, std::function<QJsonObject(const QStringList&)>>;

class RemotePlaylist : public QObject
{
  Q_OBJECT

public:
    explicit RemotePlaylist(const Application *app, QObject *parent = nullptr);
    ~RemotePlaylist() = default;

    const PlaylistCmdMap& sendCommandMap() const;
    QJsonObject sendAllPlaylists() const;

private:
    void sendCoverImage();
    void sendPendingPlaylist();

    const Application *app_;
    RemoteCurrentSong *currentSong_;

    PlaylistCmdMap commandMap;

    void createCommandMap();
    QJsonObject clearRemoteCurrentPlaylist(const QStringList& args);
    QJsonObject closeCurrentPlaylist(const QStringList& args);
    QJsonObject removeCurrentSongsPlaylist(const QStringList& songs);
    QJsonObject removeDuplicatesPlaylist();
    // Rename current playlist. Send command rename-playlist <new name>.
    QJsonObject renameCurrentPlaylist(const QStringList& args);
    // Shuffle all playlists.
    QJsonObject shuffleAllPlaylists();
    // Shuffle single playlist.
    QJsonObject shuffleSinglePlaylist(const QStringList& args);
    // Delete current playlist.
    QJsonObject deleteCurrentDevicePlaylist(const QStringList& args);
    // Make playlist a favourite or not.
    QJsonObject setFavouritePlaylist(const QStringList& args);
    // Set the playlist as current. Send set-current-playlist <playlist ID>.
    QJsonObject setCurrentPlaylist(const QStringList& args);
    // Send active playlist on remote to server.
    QJsonObject receiveRemoteActive(const QStringList& args);
    // Send requested playlist to remote.
    QJsonObject sendRequestedPLaylist(const QStringList& args);
    // Make playlists to send back to device.
    QJsonObject makeAllPlaylists() const;
    // QJsonObject makeSinglePlaylist(const int id);
    QJsonObject makePlaylistData(const int id) const;
    // Send number of arguments needed back to remote.
    QJsonObject wrongNumArgs(const int num);

    void testUrl() const; // Delete after testing URL.



Q_SIGNALS:
    void sendResponse(const QJsonObject& response);
    void clearPlaylist();
    void closePlaylist(const int id);
    void deletePlaylist(const int id);
    void remoteFavouritePlaylist(const int id, bool isFavourite);
    void serverFavouritePlaylist(const int id, bool isFavourite);
    void removeCurrentSong();
    void removeDuplicates();
    void removeItemsWithoutUndo(const int id, const QList<int> &indices);
    void remoteRenamePlaylist(const int id, const QString& name);
    void setActivePlaylist(const int id);
    void setCurrentPlaylistSignal(const int id);
    void shufflePlaylist();



private Q_SLOTS:
    // If playlist is changed on server, send updated playlist.
    void closeServerPlaylist(const int id);
    void deleteServerPlaylist(const int id);
    void favouriteServerPlaylist(const int id, bool favourite);
    void serverRenamePlaylist(const int id, const QString& name);
    void activeChanged(const int id); // Sends new active playlist id.
    void sendPlaylistData(const int id);
};

#endif // REMOTEPLAYLIST_H
