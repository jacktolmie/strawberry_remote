#include "remotealbumart.h"
#include "remotejsoncreator.h"
#include "remotetypes.h"

using namespace RemoteTypes;

RemoteAlbumArt::RemoteAlbumArt(QObject *parent)
    : QObject{parent}
{}

QJsonObject RemoteAlbumArt::makeAlbumArtBySong(const Song& song) const{

    QFile file(song.art_manual().toLocalFile());

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return RemoteJsonCreator::createResponse({
            field(MessageType::ERROR, toString(MessageType::ERROR)),
            field(Error::ERROR, toString(Error::COVER_NOT_FOUND))
        });
    }

    QByteArray imageData{file.readAll()};
    file.close();

    return RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Arguments::COVER_IMAGE)),
        field(Arguments::NAME, QFileInfo(song.art_manual().toLocalFile()).fileName()),
        field(Arguments::COVER_IMAGE, QString::fromLatin1(imageData.toBase64()))
    });
}

QJsonObject RemoteAlbumArt::makeAlbumArtByName(const QString& coverArt) const {

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

    return RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Arguments::COVER_IMAGE)),
        field(Arguments::NAME, coverArt),
        field(Arguments::COVER_IMAGE, QString::fromLatin1(imageData.toBase64()))
    });
}