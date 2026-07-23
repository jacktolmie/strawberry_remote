#include "remotealbumart.h"
#include "remotejsoncreator.h"
#include "remotetypes.h"

using namespace RemoteTypes;

RemoteAlbumArt::RemoteAlbumArt(QObject *parent)
    : QObject{parent}
{}

QJsonObject RemoteAlbumArt::makeAlbumArtBySong(const Song& song) const{
    qInfo()<<"RemoteAlbumArg by song called";
    QFile file(song.art_manual().toLocalFile());

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return RemoteJsonCreator::createResponse({
            field(MessageType::ERROR, toString(MessageType::ERROR)),
            field(Error::ERROR, toString(Error::COVER_NOT_FOUND))
        });
    }

    QByteArray imageData{file.readAll()};
    file.close();
    // QString base64Image{QString::fromLatin1(imageData.toBase64())};

    return RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::COVER_IMAGE)),
        field(Arguments::NAME, QFileInfo(song.art_manual().toLocalFile()).fileName()),
        field(Arguments::COVER_IMAGE, QString::fromLatin1(imageData.toBase64()))// base64Image)
    });
}

QJsonObject RemoteAlbumArt::makeAlbumArtByName(const QString& coverArt) const {
    qInfo()<<"RemoteAlbumArg by name called";
    QFile file{coverArt};
    QFileInfo fileInfo{coverArt};

    if(!fileInfo.exists() || !fileInfo.isFile() || !file.open(QIODevice::ReadOnly)){
        return RemoteJsonCreator::createResponse({
            field(MessageType::ERROR, toString(MessageType::ERROR)),
            field(Error::ERROR, toString(Error::COVER_NOT_FOUND))
        });
    }

    QByteArray imageData = file.readAll();
    file.close();

    QString base64Image{QString::fromLatin1(imageData.toBase64())};

    return RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::COVER_IMAGE)),
        field(Arguments::NAME, coverArt),
        field(Arguments::COVER_IMAGE, base64Image)
    });

    QJsonObject obj;
    obj[u"type"_s] = u"cover_image"_s;
    obj[u"filename"_s] = fileInfo.fileName();
    obj[u"data"_s] = QString::fromLatin1(imageData.toBase64());
    return obj;
}