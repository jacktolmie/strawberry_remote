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

public:
    explicit RemoteCurrentSong(const Application *app, QObject *parent = nullptr);

    // QJsonObject sendAlbumArt(const Song& song);
    QJsonObject songData(const Song& song) const;
    QJsonObject songInfoData(const Song& song) const;

    void makeAlbumArt(const Song& song);

public Q_SLOTS:
    void getCurrentSongRequest(const Song& song);

Q_SIGNALS:
    void sendCurrentSongData(const QJsonObject& song);
    void sendAlbumArt(const QJsonObject& albumArt);
};

#endif // REMOTECURRENTSONG_H
