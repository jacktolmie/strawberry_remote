#include "playlist/playlistmanager.h"
#include "constants/timeconstants.h"
#include "remotecurrentsong.h"
#include "remotetypes.h"
#include "remotejsoncreator.h"


using namespace Qt::Literals::StringLiterals;
using namespace RemoteTypes;

RemoteCurrentSong::RemoteCurrentSong(const Application *app, QObject *parent)
  : QObject{parent},
    app_(app)
{
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::CurrentSongChanged, this, &RemoteCurrentSong::getCurrentSongRequest);
}

QJsonObject RemoteCurrentSong::songData(const Song& song) const {
    QJsonObject songInfo;
    songInfo[toString(Arguments::ARTIST)] =         song.artist();
    songInfo[toString(Arguments::ALBUM)] =          song.album();
    songInfo[toString(Arguments::COVER_IMAGE)] =    QFileInfo(song.art_manual().toLocalFile()).fileName();
    songInfo[toString(Arguments::ID)] =             song.id();
    songInfo[toString(Arguments::LENGTH)] =         song.length_nanosec() / kNsecPerMsec;
    songInfo[toString(Arguments::SONG_URL)] =       song.url().toString();
    songInfo[toString(Arguments::TITLE)] =          song.PrettyTitle();

    return songInfo;
}

QJsonObject RemoteCurrentSong::songInfoData(const Song& song) const {
    return RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::SONG_INFO)),
        field(Arguments::ID, song.id()),
        field(Arguments::ARTIST, song.artist()),
        field(Arguments::ALBUM, song.album()),
        field(Arguments::COVER_IMAGE, QFileInfo(song.art_manual().toLocalFile()).fileName()),
        field(Arguments::SONG_URL, song.url().toString()),
        field(Arguments::TITLE, song.PrettyTitle()),
        field(Arguments::LENGTH, song.length_nanosec() / kNsecPerMsec)
    });
}

void RemoteCurrentSong::getCurrentSongRequest(const Song& song){
  Q_EMIT sendCurrentSongData(songInfoData(song));
}

void RemoteCurrentSong::makeAlbumArt(const Song& song){
    QFile file(song.art_manual().toLocalFile());

    if(file.open(QIODevice::ReadOnly)){
        QByteArray imageData{file.readAll()};
        file.close();
        QString base64Image{QString::fromLatin1(imageData.toBase64())};

        Q_EMIT sendAlbumArt(
            RemoteJsonCreator::createResponse({
            field(MessageType::EVENT, toString(MessageType::EVENT)),
            field(Event::EVENT, toString(Event::COVER_IMAGE)),
            field(Arguments::NAME, QFileInfo(song.art_manual().toLocalFile()).fileName()),
            field(Arguments::COVER_IMAGE, base64Image)
            })
        );
    }
}