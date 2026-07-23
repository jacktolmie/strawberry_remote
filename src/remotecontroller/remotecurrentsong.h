#ifndef REMOTECURRENTSONG_H
#define REMOTECURRENTSONG_H

#include <QObject>
#include <QJsonObject>

#include "core/application.h"
#include "core/song.h"
#include "remotealbumart.h"

class RemoteCurrentSong : public QObject
{
    Q_OBJECT

    const Application       *app_;
    const RemoteAlbumArt    albumArt;

public:
    explicit RemoteCurrentSong(const Application *app, QObject *parent = nullptr);

    QJsonObject songData(const Song& song, const int playlistId, const int index) const;
    QJsonObject songInfoData(const Song& song) const;
    QJsonObject requestAlbumArt(const QString& coverArt); //(const Song& song) const;

public Q_SLOTS:
    void getCurrentSongRequest(const Song& song);

Q_SIGNALS:
    void sendCurrentSongData(const QJsonObject& song);
};

#endif // REMOTECURRENTSONG_H
