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
    return RemoteJsonCreator::createResponse({
        field(Arguments::ID, song.id()),
        field(Arguments::ARTIST, song.artist()),
        field(Arguments::ALBUM, song.album()),
        field(Arguments::TITLE, song.PrettyTitle()),
        field(Arguments::LENGTH, song.length_nanosec() / kNsecPerMsec)
    });
}

QJsonObject RemoteCurrentSong::songInfo(const Song& song) const {
    auto obj = songData(song);
    obj.insert(u"type"_s, toString(MessageType::RESPONSE));
    obj.insert(u"response"_s, toString(Response::SONG_INFO));
    return obj;
}

void RemoteCurrentSong::getCurrentSongRequest(const Song& song){
  qInfo()<<"remotecurrentsong getcurrentsongrequest called with: " << song.PrettyTitleWithArtist();
  Q_EMIT sendCurrentSongData(songInfo(song));
}