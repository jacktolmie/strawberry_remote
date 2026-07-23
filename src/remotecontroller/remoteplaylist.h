#ifndef REMOTEPLAYLIST_H
#define REMOTEPLAYLIST_H

#include <QObject>
#include <QDebug>
#include <QTimer>

#include "core/application.h"
#include "playlist/playlistsequence.h"
#include "remotecurrentsong.h"

using PlaylistCmdMap = QMap<QString, std::function<QJsonObject(const QJsonObject&)>>;

class RemotePlaylist : public QObject
{
  Q_OBJECT

public:
    explicit RemotePlaylist(const Application *app, QObject *parent = nullptr);
    ~RemotePlaylist() = default;

    const PlaylistCmdMap& sendCommandMap() const;
    // Return current repeat mode
    QString     repeatMode() const;
    QJsonObject sendAllPlaylists() const;

private:
    void sendPendingPlaylist();

    const Application   *app_;
    RemoteCurrentSong   *currentSong_;

    QTimer *metadataTimer_;
    int pendingPlaylistId_;

    PlaylistCmdMap commandMap;

    void createCommandMap();
    QJsonObject clearRemoteCurrentPlaylist(const QJsonObject& args);
    QJsonObject closeRemoteCurrentPlaylist(const QJsonObject& args);
    QJsonObject removeCurrentSongsPlaylist(const QJsonObject& songs);
    QJsonObject removeDuplicatesPlaylist(const QJsonObject& args);
    // Rename current playlist. Send command rename-playlist <new name>.
    QJsonObject renameCurrentPlaylist(const QJsonObject& args);
    // Shuffle all playlists.
    QJsonObject shuffleAllPlaylists();
    // Shuffle single playlist.
    QJsonObject shuffleSinglePlaylist(const QJsonObject& args);
    // Delete current playlist.
    QJsonObject deleteCurrentRemotePlaylist(const QJsonObject& args);
    // Send active playlist on remote to server.
    QJsonObject receiveRemoteActive(const QJsonObject& args);
    // Send current album cover.
    QJsonObject sendCoverImage(const QJsonObject& args);
    // Make playlist a favourite or not.
    QJsonObject setFavouritePlaylist(const QJsonObject& args);
    // Set the playlist as current. Send set-current-playlist <playlist ID>.
    QJsonObject setCurrentPlaylist(const QJsonObject& args);
    // Set the repeat mode from the remote device
    QJsonObject setRepeatMode(const QJsonObject& args);
    // Send requested playlist to remote.
    QJsonObject sendRequestedPLaylist(const QJsonObject& args);
    // Make playlists to send back to device.
    QJsonObject makeAllPlaylists() const;
    // QJsonObject makeSinglePlaylist(const int id);
    QJsonObject makePlaylistData(const int id) const;
    // Send number of arguments needed back to remote.
    QJsonObject wrongArgsSent(const QString& error);  //(const int num);

Q_SIGNALS:
    void sendResponse(const QJsonObject& response);
    void clearPlaylist();
    void remoteClosedPlaylist(const int id);
    void deletePlaylist(const int id);
    void remoteFavouritePlaylist(const int id, bool isFavourite);
    void serverFavouritePlaylist(const int id, bool isFavourite);
    void removeCurrentSong();
    void removeDuplicates();
    void removeItemsWithoutUndo(const int id, const QList<int> &indices);
    void remoteRenamePlaylist(const int id, const QString& name);
    void requestAlbumArt(const QString& coverArt); //(const Song& song);
    void setActivePlaylist(const int id);
    void setCurrentPlaylistSignal(const int id);
    void setRepeatModeSignal(const PlaylistSequence::RepeatMode mode);
    void shufflePlaylist();

private Q_SLOTS:
    // If playlist is changed on server, send updated playlist.
    void activeChanged(const int id); // Sends new active playlist id.
    void closeServerPlaylist(const int id);
    void deleteServerPlaylist(const int id);
    void favouriteServerPlaylist(const int id, bool favourite);
    void onPlaylistMetadataChanged(const int id);
    void onPlaylistMetadataChangedWithQUuid(const int id, const QUuid track_id);
    void playlistItemsAdded(const int playlist_id, const QList<QUuid> &track_ids, const QUuid after_track_id);
    void playlistManagerLoaded();
    void repeatModeChanged(const PlaylistSequence::RepeatMode mode);
    void sendPlaylistData(const int id);
    void serverRenamePlaylist(const int id, const QString& name);
};

#endif // REMOTEPLAYLIST_H
