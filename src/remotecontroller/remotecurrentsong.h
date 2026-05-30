#ifndef REMOTECURRENTSONG_H
#define REMOTECURRENTSONG_H

#include <QObject>
#include <QJsonObject>

#include "core/application.h"
#include "core/song.h"
#include "covermanager/albumcoverfetcher.h"
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

public:
  explicit RemoteCurrentSong(const Application *app, QObject *parent = nullptr);

  QJsonObject songInfo(const Song& song) const;

public Q_SLOTS:


};

#endif // REMOTECURRENTSONG_H
