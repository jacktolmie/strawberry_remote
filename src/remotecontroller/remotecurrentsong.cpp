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
{}

QJsonObject RemoteCurrentSong::songInfo(const Song& song) const {

  // Create JSON object from sent song.
  return RemoteJsonCreator::createResponse({
    field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
    field(Response::RESPONSE, toString(Response::SONG_INFO)),
    field(Arguments::ID, song.id()),
    field(Arguments::ARTIST, song.artist()),
    field(Arguments::ALBUM, song.album()),
    field(Arguments::TITLE, song.PrettyTitle()),
    field(Arguments::LENGTH, song.length_nanosec() / kNsecPerMsec)
// coverFinder->findRemoteUrlForSong(song); // Find way to get Album URL image
  });
}