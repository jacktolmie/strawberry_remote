#include "playlist/playlistmanager.h"
#include "remotecurrentsong.h"
#include "constants/timeconstants.h"
#include "remotecontroller/remotetypes.h"
#include "remotecontroller/remotejsoncreator.h"
#include <QTimer>

using namespace Qt::Literals::StringLiterals;
using namespace RemoteTypes;

RemoteCurrentSong::RemoteCurrentSong(const Application *app, QObject *parent)
  : QObject{parent},
    app_(app)
{
  QObject::connect(&*app_->playlist_manager(), &PlaylistManager::CurrentSongChanged, this, &RemoteCurrentSong::getCurrentSongRequest);
}

QJsonObject RemoteCurrentSong::songData(const Song& song) const {
    qInfo()<<"songid: " << song.id();
    QJsonObject songInfo;
    songInfo[toString(Arguments::ID)] = song.id();
    songInfo[toString(Arguments::SONG_URL)] = song.url().toString();
    songInfo[toString(Arguments::ARTIST)] = song.artist();
    songInfo[toString(Arguments::ALBUM)] = song.album();
    songInfo[toString(Arguments::TITLE)] = song.PrettyTitle();
    songInfo[toString(Arguments::LENGTH)] = song.length_nanosec() / kNsecPerMsec;
    return songInfo;
}

// Full event envelope - use this for standalone song_info events
QJsonObject RemoteCurrentSong::songInfoData(const Song& song) const {
    return RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::SONG_INFO)),
        field(Arguments::ID, song.id()),
        field(Arguments::ARTIST, song.artist()),
        field(Arguments::ALBUM, song.album()),
        field(Arguments::TITLE, song.PrettyTitle()),
        field(Arguments::LENGTH, song.length_nanosec() / kNsecPerMsec)
    });
}

void RemoteCurrentSong::getCurrentSongRequest(const Song& song){
  Q_EMIT sendCurrentSongData(songData(song));
}