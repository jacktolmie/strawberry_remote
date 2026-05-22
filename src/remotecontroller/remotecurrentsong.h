#ifndef REMOTECURRENTSONG_H
#define REMOTECURRENTSONG_H

#include <QObject>
#include <QJsonObject>

#include "core/application.h"
#include "covermanager/albumcoverloaderresult.h"
#include "covermanager/coversearchstatistics.h"
#include "core/song.h"

#include "covermanager/albumcoverfetcher.h"
#include "covermanager/albumcoverimageresult.h"
#include "covermanager/coversearchstatistics.h"
#include "covermanager/currentalbumcoverloader.h"
#include "remotecontroller/remoteguivalues.h"

class AlbumCoverFetcher; // Needed?
class CurrentAlbumCoverLoader;

class RemoteCurrentSong : public QObject
{
  Q_OBJECT

  const Application *app_;
  const AlbumCoverFetcher *coverFetcher;
  const RemoteGuiValues *coverFinder;

  QUrl albumImageUrl;
  QUrl songLyricsUrl;

public:
  explicit RemoteCurrentSong(const Application *app, QObject *parent = nullptr);

  QJsonObject songInfo(Song song);

public Q_SLOTS:
  // void getAlbumURL(const Song &song, const AlbumCoverLoaderResult &result);
  void getAlbumThumbnail(const Song &song, const QUrl &thumbnail_uri);//, const QImage &image);
  // void onNewCoverFetched(quint64 request_id, const AlbumCoverImageResult &result, const CoverSearchStatistics &statistics);

};

#endif // REMOTECURRENTSONG_H
