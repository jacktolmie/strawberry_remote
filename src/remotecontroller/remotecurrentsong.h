#ifndef REMOTECURRENTSONG_H
#define REMOTECURRENTSONG_H

#include <QObject>
#include <QJsonObject>

#include "core/application.h"
#include "core/song.h"
#include "covermanager/albumcoverfetcher.h"

class AlbumCoverFetcher; // Needed?
class CurrentAlbumCoverLoader;

class RemoteCurrentSong : public QObject
{
  Q_OBJECT

  const Application *app_;
  // const AlbumCoverFetcher *coverFetcher;

  QUrl albumImageUrl;

public:
  explicit RemoteCurrentSong(const Application *app, QObject *parent = nullptr);

  QJsonObject songData(const Song& song) const;

public Q_SLOTS:
  void getCurrentSongRequest(const Song& song);

Q_SIGNALS:
  void sendCurrentSongData(const QJsonObject& song);


};

#endif // REMOTECURRENTSONG_H
