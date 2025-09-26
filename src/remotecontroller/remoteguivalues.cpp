#include "remoteguivalues.h"
#include "core/logging.h"
#include "core/player.h"
#include "playlist/playlistmanager.h"
#include "core/song.h"
// RemoteGuiValues::RemoteGuiValues(QObject *parent)
RemoteGuiValues::RemoteGuiValues(Application *app, QObject *parent)
  : QObject{parent},
    app_{app}{
  // connect()
}

void RemoteGuiValues::getUpdates(){

}

// void RemoteGuiValues::findRemoteUrlForSong(const Song &song)
// {
//     if (!song.is_valid() || song.artist().isEmpty() || song.album().isEmpty()) {
//         qDebug() << "Cannot search for cover, song metadata is incomplete.";
//         return;
//     }

//     qDebug() << "Remote Starting on-demand online search for:" << song.artist() << "-" << song.album();

//     // Call the fetcher to start the asynchronous online search.
//     // The 'false' parameter indicates this is not part of a batch job.
//     quint64 request_id = m_fetcher->FetchAlbumCover(song.artist(), song.album(), song.title(), false);

//     // Store the request ID and the song so we can match them up when the result comes back.
//     m_pendingRequests.insert(request_id, song);
// }

// void RemoteGuiValues::onFetcherResult(quint64 request_id, const AlbumCoverImageResult &result, const CoverSearchStatistics &statistics)
// {
//     // Check if this result is for one of our pending requests.
//     if (!m_pendingRequests.contains(request_id)) {
//         // This result is for some other part of the app, ignore it.
//         return;
//     }

//     // It's for us! Get the original song and then remove the request from our list.
//     Song originalSong = m_pendingRequests.take(request_id);
//     QUrl remoteUrl = result.cover_url;

//     if (remoteUrl.isValid() && !remoteUrl.isLocalFile()) {
//         qDebug() << "SUCCESS! Found remote URL for" << originalSong.title() << "->" << remoteUrl;

//         // Emit our custom signal with the paired data!
//         Q_EMIT RemoteGuiValues::remoteUrlFound(originalSong, remoteUrl);
//     }
// }
