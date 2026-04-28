#include "remotecurrentsong.h"
#include "constants/timeconstants.h"
#include "remotecontroller/remoteguivalues.h"

using namespace Qt::Literals::StringLiterals;

RemoteCurrentSong::RemoteCurrentSong(Application *app, QObject *parent)
  : QObject{parent},
    app_(app)
{
  // connect(&*app_->current_albumcover_loader(), &CurrentAlbumCoverLoader::AlbumCoverLoaded, this, &RemoteCurrentSong::getAlbumURL);
  // coverFetcher = new AlbumCoverFetcher(app_->cover_providers(), app_->network(), this);
  // AlbumCoverFetcher* sharedCoverFetcher = app_->;
  // coverFinder = new RemoteGuiValues(coverFetcher, this);
  // connect(coverFinder, &RemoteGuiValues::remoteUrlFound, this, &RemoteCurrentSong::getAlbumThumbnail);
}

QJsonObject RemoteCurrentSong::songInfo(Song song){
  // Create JSON object from sent song.
  QJsonObject songData;
  songData[u"id"_s] =      song.id();
  songData[u"artist"_s] =  song.artist();
  songData[u"album"_s] =   song.album();
  songData[u"title"_s] =   song.PrettyTitle();
  songData[u"length"_s] =  song.length_nanosec() / kNsecPerMsec;
  // coverFinder->findRemoteUrlForSong(song);
  return songData;
}

void RemoteCurrentSong::getAlbumThumbnail(const Song &song, const QUrl &thumbnail_uri)//, [[maybe_unused]] const QImage &image){
{
  qDebug() <<"Remote Thumbnail called with url: "<< thumbnail_uri << " for song: " << song.PrettyTitleWithArtist();
}

// void RemoteCurrentSong::getAlbumURL(const Song &song, const AlbumCoverLoaderResult &result){
//   qDebug() <<"Remote signal emitted for Album URL with: " << song.url();
//  if (!result.success) {
//         // Cover loading failed, nothing to do.
//         return;
//     }

//     QUrl coverUrl;

//     // Determine the source of the cover from the result type
//     switch (result.type) {
//         case AlbumCoverLoaderResult::Type::Automatic:
//             // This is a cover found online by the fetcher.
//             // The URL is stored in the song's 'art_automatic' property.
//             coverUrl = song.art_automatic();
//             break;

//         case AlbumCoverLoaderResult::Type::Manual:
//             // This is a cover set manually by the user.
//             // The URL is stored in the song's 'art_manual' property.
//             coverUrl = song.art_manual();
//             break;

//         case AlbumCoverLoaderResult::Type::Embedded:
//             // The cover is embedded in the audio file itself.
//             // There is no external URL to forward.
//             qDebug() << "Cover is embedded for song:" << song.title() << ". No URL available.";
//             return; // Exit early

//         case AlbumCoverLoaderResult::Type::Unset:
//         case AlbumCoverLoaderResult::Type::None:
//             // No cover was found or the user has set "no cover".
//             qDebug() << "No cover found for song:" << song.title();
//             return; // Exit early
//     }

//     // At this point, coverUrl holds the URL you need.
//     // Now, check if it's a remote URL that can be forwarded.
//     if (coverUrl.isValid() && !coverUrl.isLocalFile()) {
//         qDebug() << "Found remote cover art URL for" << song.title() << ":" << coverUrl.toString();

//         // This is a remote URL like http:// or https://
//         // You can now forward coverUrl.toString() to your remote device!
//         // sendUrlToRemoteDevice(coverUrl.toString());

//     } else if (coverUrl.isLocalFile()) {
//         qDebug() << "Cover is a local file, not a remote URL:" << coverUrl.toLocalFile();
//         // You could potentially handle this by setting up a local web server
//         // to serve the file, but it's not a simple forward.
//     } else {
//         qDebug() << "Could not determine a valid remote URL.";
//     }
// }


// void RemoteCurrentSong::onNewCoverFetched(quint64 request_id, const AlbumCoverImageResult &result, const CoverSearchStatistics &statistics){

//   // QObject::connect(&*coverFetcher, &AlbumCoverFetcher::AlbumCoverFetched,
//   //                    this,         &RemoteCurrentSong::onNewCoverFetched);
//     // The result object contains the remote URL directly.
//     QUrl remoteUrl = result.cover_url;

//     if (remoteUrl.isValid() && !remoteUrl.isLocalFile()) {
//         qDebug() << "SUCCESS! Newly fetched remote cover URL is:" << remoteUrl.toString();

//         // You can now forward this URL to your remote device.
//         // sendUrlToRemoteDevice(remoteUrl.toString());
//     }
// }
