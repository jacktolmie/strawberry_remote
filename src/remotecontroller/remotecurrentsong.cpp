#include "playlist/playlistmanager.h"
#include "constants/timeconstants.h"
#include "remotecurrentsong.h"
#include "remotetypes.h"
#include "remotejsoncreator.h"


using namespace Qt::Literals::StringLiterals;
using namespace RemoteTypes;

RemoteCurrentSong::RemoteCurrentSong(const Application *app, QObject *parent)
  : QObject{parent},
    app_(app),
    albumArt(RemoteAlbumArt(this))
{
    QObject::connect(&*app_->playlist_manager(), &PlaylistManager::CurrentSongChanged, this, &RemoteCurrentSong::getCurrentSongRequest);
}

// This is used by remoteplaylist to create songs for playlists
QJsonObject RemoteCurrentSong::songData(const Song& song, const int playlistId, const int index) const {
    QJsonObject songInfo;
    songInfo[toString(Arguments::ARTIST)] =         song.artist();
    songInfo[toString(Arguments::ALBUM)] =          song.album();
    songInfo[toString(Arguments::COVER_IMAGE)] =    song.art_manual().toLocalFile();
    songInfo[toString(Arguments::ID)] =             song.id();
    songInfo[toString(PlaylistData::LENGTH)] =      song.length_nanosec() / kNsecPerMsec;
    songInfo[toString(PlaylistData::PLAYLIST_ID)] = playlistId,
    songInfo[toString(PlaylistData::POSITION)] =    index,
    songInfo[toString(PlaylistData::SONG_URL)] =    song.url().toString();
    songInfo[toString(Arguments::TITLE)] =          song.PrettyTitle();

    return songInfo;
}

// This is used to send single song information when song is changed.
QJsonObject RemoteCurrentSong::songInfoData(const Song& song) const {

    return RemoteJsonCreator::createResponse({
        field(MessageType::EVENT,       toString(MessageType::EVENT)),
        field(Event::EVENT,             toString(Event::SONG_INFO)),
        field(Arguments::ID,            song.id()),
        field(Arguments::ARTIST,        song.artist()),
        field(Arguments::ALBUM,         song.album()),
        field(Arguments::COVER_IMAGE,   QFileInfo(song.art_manual().toLocalFile()).fileName()),
        field(PlaylistData::PLAYLIST_ID, app_->playlist_manager()->active()->id()),
        field(PlaylistData::POSITION,   app_->playlist_manager()->active()->current_row()),
        field(PlaylistData::SONG_URL,   song.url().toString()),
        field(Arguments::TITLE,         song.PrettyTitle()),
        field(PlaylistData::LENGTH,     song.length_nanosec() / kNsecPerMsec)
    });
}

void RemoteCurrentSong::getCurrentSongRequest(const Song& song){
  Q_EMIT sendCurrentSongData(songInfoData(song));
}

QJsonObject RemoteCurrentSong::requestAlbumArt(const QString& coverArt){
    return albumArt.makeAlbumArtByName(coverArt);
}