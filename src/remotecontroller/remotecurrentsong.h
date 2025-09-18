#ifndef REMOTECURRENTSONG_H
#define REMOTECURRENTSONG_H

#include <QObject>
#include <QJsonObject>

#include "core/application.h"
#include "covermanager/albumcoverloaderresult.h"
#include "core/song.h"

class RemoteCurrentSong : public QObject
{
  Q_OBJECT

  Application *app_;

  QUrl albumImageUrl;
  QUrl songLyricsUrl;

public:
  explicit RemoteCurrentSong(Application *app, QObject *parent = nullptr);

  QJsonObject songInfo(Song song);

public Q_SLOTS:
  void getAlbumURL(const Song &song, const AlbumCoverLoaderResult &result);
  void getAlbumThumbnail(const Song &song, const QUrl &thumbnail_uri, const QImage &image);

};

#endif // REMOTECURRENTSONG_H
