#include <QObject>
#include "remotetypes.h"

using namespace Qt::Literals::StringLiterals;

namespace RemoteTypes {

QString toString(Arguments value) {
    switch (value) {
        case Arguments::ACTIVE_PLAYLIST:    return u"active_playlist"_s;
        case Arguments::ALBUM:              return u"album"_s;
        case Arguments::ARGUMENT:           return u"argument"_s;
        case Arguments::ARGUMENTS:          return u"arguments"_s;
        case Arguments::ARTIST:             return u"artist"_s;
        case Arguments::COMMAND:            return u"command"_s;
        case Arguments::COVER_IMAGE:        return u"cover_image"_s;
        case Arguments::CURRENT_PLAYLIST:   return u"current_playlist"_s;
        case Arguments::CURRENT_SONG:       return u"current_song"_s;
        case Arguments::FAVOURITE:          return u"favourite"_s;
        case Arguments::ID:                 return u"id"_s;
        case Arguments::IS_FAVOURITE:       return u"is_favourite"_s;
        case Arguments::LENGTH:             return u"length"_s;
        case Arguments::NAME:               return u"name"_s;
        case Arguments::NONCE:              return u"nonce"_s;
        case Arguments::REQUIRED:           return u"required"_s;
        case Arguments::ROW:                return u"row"_s;
        case Arguments::PAUSED:             return u"paused"_s;
        case Arguments::PLAYLIST:           return u"playlist"_s;
        case Arguments::PLAYLISTS:          return u"playlists"_s;
        case Arguments::PLAYING:            return u"playing"_s;
        case Arguments::SONGS:              return u"songs"_s;
        case Arguments::STOPPED:            return u"stopped"_s;
        case Arguments::TIME:               return u"time"_s;
        case Arguments::TITLE:              return u"title"_s;
        case Arguments::TRACK_ID:           return u"track_id"_s;
        case Arguments::SONG_URL:           return u"song_url"_s;
        case Arguments::VOLUME:             return u"volume"_s;
    }
    Q_UNREACHABLE();
}

QString toString(Auth value) {
    switch (value) {
        case Auth::AUTH:          return u"auth"_s;
        case Auth::AUTH_FAILED:   return u"auth_failed"_s;
        case Auth::AUTH_SUCCESS:  return u"auth_success"_s;
        case Auth::CHALLENGE:     return u"challenge"_s;
        case Auth::ERROR:         return u"error"_s;
    }
    Q_UNREACHABLE();
}

QString toString(Error value) {
    switch (value) {
        case Error::COMMAND_NOT_FOUND:                  return u"command_not_found"_s;
        case Error::COVER_NOT_FOUND:                    return u"cover_not_found"_s;
        case Error::ERROR:                              return u"error"_s;
        case Error::WRONG_NUMBER_ARGUMENTS_PASSED:      return u"wrong_number_arguments_passed"_s;
        case Error::PLAYLIST_NOT_CLEARED:               return u"playlist_not_cleared"_s;
        case Error::PLAYLIST_NOT_CLOSED:                return u"playlist_not_closed"_s;
        case Error::PLAYLIST_NOT_FOUND:                 return u"playlist_not_found"_s;
        case Error::WRONG_ARGUMENT_SENT:                return u"wrong_argument_sent"_s;
    }
    Q_UNREACHABLE();
}

QString toString(Event value) {
    switch (value) {
        case Event::EVENT:                      return u"event"_s;
        case Event::ACTIVE_PLAYLIST:            return u"active_playlist"_s;
        case Event::CLOSED_PLAYLIST_WITH_ID:    return u"closed_playlist_with_id"_s;
        case Event::COVER_IMAGE:                return u"cover_image"_s;
        case Event::DELETE_PLAYLIST:            return u"delete_playlist"_s;
        case Event::FAVOURITE_PLAYLIST:         return u"favourite_playlist"_s;
        case Event::GUI_UPDATES:                return u"gui_updates"_s;
        case Event::MAKE_ALL_PLAYLISTS:         return u"make_all_playlists"_s;
        case Event::MAKE_PLAYLIST:              return u"make_playlist"_s;
        case Event::NEXT:                       return u"next"_s;
        case Event::NEW_PLAYLIST:               return u"new_playlist"_s;
        case Event::PAUSE:                      return u"pause"_s;
        case Event::PLAY:                       return u"play"_s;
        case Event::PREVIOUS:                   return u"previous"_s;
        case Event::RENAME_PLAYLIST:            return u"rename_playlist"_s;
        case Event::SEEK_BACKWARD:              return u"seek_backward"_s;
        case Event::SEEK_FORWARD:               return u"seek_forward"_s;
        case Event::SEEK_TO:                    return u"seek_to"_s;
        case Event::SENT_ACTIVE_PLAYLIST:       return u"sent_active_playlist"_s;
        case Event::SONG_CHANGED:               return u"song_changed"_s;
        case Event::SONG_INFO:                  return u"song_info"_s;
        case Event::STOP:                       return u"stop"_s;
        case Event::VOLUME_CHANGED:             return u"volume_changed"_s;
    }
    Q_UNREACHABLE();
}

QString toString(MessageType value) {
    switch (value) {
        case MessageType::AUTH:     return u"auth"_s;
        case MessageType::ERROR:    return u"error"_s;
        case MessageType::EVENT:    return u"event"_s;
        case MessageType::RESPONSE: return u"response"_s;
    }
    Q_UNREACHABLE();
}

QString toString(Response value) {
    switch (value) {
        case Response::CLEARED_PLAYLIST:                return u"cleared_playlist"_s;
        case Response::DELETED_PLAYLIST_WITH_ID:        return u"deleted_playlist_with_id"_s;
        case Response::IS_PLAYLIST_A_FAVOURITE:         return u"is_playlist_a_favourite"_s;
        case Response::PLAYLIST_CLOSED:                 return u"playlist_closed"_s;
        case Response::REMOVED_DUPLICATES_FROM_PLAYLIST: return u"removed_duplicates_from_playlist"_s;
        case Response::RENAME_PLAYLIST:                 return u"rename_playlist"_s;
        case Response::REMOVED_SONGS_FROM_PLAYLIST:     return u"removed_songs_from_playlist"_s;
        case Response::RESPONSE:                        return u"response"_s;
        case Response::RUNNING_COMMAND:                 return u"running_command"_s;
        case Response::SENT_ALBUM_COVER:                return u"send_album_cover"_s;
        case Response::SEND_REQUESTED_PLAYLIST:         return u"send_requested_playlist"_s;
        case Response::SET_CURRENT_PLAYLIST_TO:         return u"set_current_playlist_to"_s;
        case Response::SHUFFLED_PLAYLIST:               return u"shuffled_playlist"_s;
        case Response::SHUFFLED_ALL_PLAYLISTS:          return u"shuffled_all_playlists"_s;
    }
    Q_UNREACHABLE();
}

} // namespace RemoteTypes
