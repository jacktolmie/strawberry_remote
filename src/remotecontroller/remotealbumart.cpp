#include "remotealbumart.h"
#include "remotejsoncreator.h"
#include "remotetypes.h"

using namespace RemoteTypes;

RemoteAlbumArt::RemoteAlbumArt(QObject *parent)
    : QObject{parent}
{}

QJsonObject RemoteAlbumArt::makeAlbumArt(const Song& song) const{
    QFile file(song.art_manual().toLocalFile());

    if(file.open(QIODevice::ReadOnly)){
        QByteArray imageData{file.readAll()};
        file.close();
        QString base64Image{QString::fromLatin1(imageData.toBase64())};

        return RemoteJsonCreator::createResponse({
            field(MessageType::EVENT, toString(MessageType::EVENT)),
            field(Event::EVENT, toString(Event::COVER_IMAGE)),
            field(Arguments::NAME, QFileInfo(song.art_manual().toLocalFile()).fileName()),
            field(Arguments::COVER_IMAGE, base64Image)
        });
    }else{
        // Send empty QJsonObject back to caller.
        return QJsonObject();
    }

}

void RemoteAlbumArt::requestAlbumArt(const Song& song){

}

// Q_EMIT sendAlbumArt(RemoteJsonCreator::createResponse({
//         field(MessageType::EVENT, toString(MessageType::EVENT)),
//         field(Event::EVENT, toString(Event::COVER_IMAGE)),
//         field(Arguments::NAME, QFileInfo(song.art_manual().toLocalFile()).fileName()),
//         field(Arguments::COVER_IMAGE, base64Image)
//     }));

// return RemoteJsonCreator::createResponse({
//     field(MessageType::ERROR, toString(MessageType::ERROR)),
//     field(Error::COVER_NOT_FOUND, toString(Error::COVER_NOT_FOUND)),
//     field(Arguments::SONG_URL, song.url().toString())
// });