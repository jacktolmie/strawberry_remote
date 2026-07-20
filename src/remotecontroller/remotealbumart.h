#ifndef REMOTEALBUMART_H
#define REMOTEALBUMART_H

#include <QObject>
#include "core/song.h"

class RemoteAlbumArt : public QObject
{
    Q_OBJECT
public:
    explicit RemoteAlbumArt(QObject *parent = nullptr);

    QJsonObject makeAlbumArt(const Song& song) const;

public Q_SLOTS:
    void requestAlbumArt(const Song& song);

Q_SIGNALS:
    void sendAlbumArt(const QJsonObject& albumArt);
};

#endif // REMOTEALBUMART_H
