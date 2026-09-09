#include <QObject>
#include "remotetypes.h"

using namespace Qt::Literals::StringLiterals;

namespace RemoteTypes {

QString toString(Arguments value) {
    switch (value) {
        case Arguments::ALBUM:              return u"album"_s;
        case Arguments::ARGUMENT:           return u"argument"_s;
        case Arguments::ARGUMENTS:          return u"arguments"_s;
        case Arguments::ARTIST:             return u"artist"_s;
        case Arguments::COMMAND:            return u"command"_s;
        case Arguments::COVER_ART:          return u"cover_art"_s;
        case Arguments::COVER_IMAGE:        return u"cover_image"_s;
        case Arguments::CURRENT_SONG:       return u"current_song"_s;
        case Arguments::EMPTY:              return u""_s;
        case Arguments::EMPTY_FIELDS:       return u"empty_fields"_s;
        case Arguments::ERROR:              return u"error"_s;
        case Arguments::ID:                 return u"id"_s;
        case Arguments::NAME:               return u"name"_s;
        case Arguments::NONCE:              return u"nonce"_s;
        case Arguments::PAUSED:             return u"paused"_s;
        case Arguments::PLAYING:            return u"playing"_s;
        case Arguments::PROOF:              return u"proof"_s;
        case Arguments::REQUIRED:           return u"required"_s;
        case Arguments::STOPPED:            return u"stopped"_s;
        case Arguments::TIME:               return u"time"_s;
        case Arguments::TITLE:              return u"title"_s;
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
        case Event::CLOSED_PLAYLIST_WITH_ID:    return u"closed_playlist_with_id"_s;
        case Event::GUI_UPDATES:                return u"gui_updates"_s;
        case Event::MAKE_ALL_PLAYLISTS:         return u"make_all_playlists"_s;
        case Event::MAKE_PLAYLIST:              return u"make_playlist"_s;
        case Event::MUSIC_TOTALS:               return u"music_totals"_s;
        case Event::MUTE:                       return u"mute"_s;
        case Event::NEXT:                       return u"next"_s;
        case Event::NEW_PLAYLIST:               return u"new_playlist"_s;
        case Event::PAUSE:                      return u"pause"_s;
        case Event::PLAY:                       return u"play"_s;
        case Event::PLAY_PAUSE:                 return u"play_pause"_s;
        case Event::PREVIOUS:                   return u"previous"_s;
        case Event::RADIO_STATIONS:             return u"radio_stations"_s;
        case Event::REPEAT_MODE:                return u"repeat_mode"_s;
        case Event::RENAME_PLAYLIST:            return u"rename_playlist"_s;
        case Event::RESTART_OR_PREVIOUS:        return u"restart_or_previous"_s;
        case Event::SEEK_BACKWARD:              return u"seek_backward"_s;
        case Event::SEEK_FORWARD:               return u"seek_forward"_s;
        case Event::SEEK_TO:                    return u"seek_to"_s;
        case Event::SENT_ACTIVE_PLAYLIST:       return u"sent_active_playlist"_s;
        case Event::SHUFFLE_MODE:               return u"shuffle_mode"_s;
        case Event::SONG_CHANGED:               return u"song_changed"_s;
        case Event::SONG_INFO:                  return u"song_info"_s;
        case Event::STOP:                       return u"stop"_s;
        case Event::STOP_AFTER_CURRENT:         return u"stop_after_current"_s;
        case Event::VOLUME:                     return u"volume"_s;
        case Event::VOLUME_CHANGED:             return u"volume_changed"_s;
        case Event::VOLUME_DOWN:                return u"volume_down"_s;
        case Event::VOLUME_UP:                  return u"volume_up"_s;
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

QString toString(PlaylistCommandMap value) {
    switch (value) {
        case PlaylistCommandMap::CLEAR_PLAYLIST:                return u"clear_playlist"_s;
        case PlaylistCommandMap::CLOSE_PLAYLIST:                return u"close_playlist"_s;
        case PlaylistCommandMap::DELETE_PLAYLIST:               return u"delete_playlist"_s;
        case PlaylistCommandMap::FAVOURITE_PLAYLIST:            return u"favourite_playlist"_s;
        case PlaylistCommandMap::REMOTE_CHANGED_PLAYLIST:       return u"remote_changed_playlist"_s;
        case PlaylistCommandMap::REMOTE_SENT_ACTIVE:            return u"remote_sent_active"_s;
        case PlaylistCommandMap::REMOVE_DUPLICATES_PLAYLIST:    return u"remove_duplicates_playlist"_s;
        case PlaylistCommandMap::REMOVE_SONGS_PLAYLIST:         return u"remove_songs_playlist"_s;
        case PlaylistCommandMap::REMOVE_UNAVAILABLE_SONGS:      return u"remove_unavailable_songs"_s;
        case PlaylistCommandMap::RENAME_PLAYLIST:               return u"rename_playlist"_s;
        case PlaylistCommandMap::REPEAT_MODE:                   return u"repeat_mode"_s;
        case PlaylistCommandMap::SEND_ACTIVE_PLAYLIST_SONG:     return u"send_active_playlist_song"_s;
        case PlaylistCommandMap::SEND_ALL_PLAYLISTS:            return u"send_all_playlists"_s;
        case PlaylistCommandMap::SEND_PLAYLIST:                 return u"send_playlist"_s;
        case PlaylistCommandMap::REQUEST_COVER:                 return u"request_cover"_s;
        case PlaylistCommandMap::SET_CURRENT_PLAYLIST:          return u"set_current_playlist"_s;
        case PlaylistCommandMap::SHUFFLE_CURRENT_PLAYLIST:      return u"shuffle_current_playlist"_s;
        case PlaylistCommandMap::SHUFFLE_MODE:                  return u"shuffle_mode"_s;

    }
}
QString toString(PlaylistData value) {
    switch (value) {

        case PlaylistData::ACTIVE_PLAYLIST:     return u"active_playlist"_s;
        case PlaylistData::ALL:                 return u"all"_s;
        case PlaylistData::CURRENT_PLAYLIST:    return u"current_playlist"_s;
        case PlaylistData::FAVOURITE:           return u"favourite"_s;
        case PlaylistData::FROM_INDEX:          return u"from_index"_s;
        case PlaylistData::GROUPING:            return u"grouping"_s;
        case PlaylistData::IS_FAVOURITE:        return u"is_favourite"_s;
        case PlaylistData::LENGTH:              return u"length"_s;
        case PlaylistData::OFF:                 return u"off"_s;
        case PlaylistData::PLAYLIST:            return u"playlist"_s;
        case PlaylistData::PLAYLIST_ID:         return u"playlist_id"_s;
        case PlaylistData::PLAYLISTS:           return u"playlists"_s;
        case PlaylistData::PLAYLIST_LENGTH:     return u"playlist_length"_s;
        case PlaylistData::PLAYLIST_SIZE:       return u"playlist_size"_s;
        case PlaylistData::POSITION:            return u"position"_s;
        case PlaylistData::REPEAT_INTRO:        return u"intro"_s;
        case PlaylistData::REPEAT_MODE:         return u"repeat_mode"_s;
        case PlaylistData::REPEAT_ONEBYONE:     return u"stop"_s;
        case PlaylistData::REPEAT_PLAYLIST:     return u"playlist"_s;
        case PlaylistData::REPEAT_TRACK:        return u"track"_s;
        case PlaylistData::ROW:                 return u"row"_s;
        case PlaylistData::SHUFFLE_MODE:        return u"shuffle_mode"_s;
        case PlaylistData::SONGS:               return u"songs"_s;
        case PlaylistData::SONGS_LIST:          return u"songs_list"_s;
        case PlaylistData::SONG_INDEX:          return u"song_index"_s;
        case PlaylistData::TO_INDEX:            return u"to_index"_s;
        case PlaylistData::TOTAL_ALBUMS:        return u"total_albums"_s;
        case PlaylistData::TOTAL_ARTISTS:       return u"total_artists"_s;
        case PlaylistData::TOTAL_SONGS:         return u"total_songs"_s;
        case PlaylistData::TRACK:               return u"track"_s;
        case PlaylistData::TRACK_ID:            return u"track_id"_s;
        case PlaylistData::SONG_URL:            return u"song_url"_s;
    }
    Q_UNREACHABLE();
}

QString toString(Response value) {
    switch (value) {
        case Response::CLEARED_PLAYLIST:                return u"cleared_playlist"_s;
        case Response::CURRENT_SONG:                    return u"current_song"_s;
        case Response::DELETED_PLAYLIST_WITH_ID:        return u"deleted_playlist_with_id"_s;
        case Response::IS_PLAYLIST_A_FAVOURITE:         return u"is_playlist_a_favourite"_s;
        case Response::PLAYLIST_CHANGED:                return u"playlist_changed"_s;
        case Response::PLAYLIST_CLOSED:                 return u"playlist_closed"_s;
        case Response::REMOVED_DUPLICATES_FROM_PLAYLIST: return u"removed_duplicates_from_playlist"_s;
        case Response::REMOVED_UNAVAILABLE_SONGS:       return u"removed_unavailable_songs"_s;
        case Response::RENAME_PLAYLIST:                 return u"rename_playlist"_s;
        case Response::REMOVED_SONGS_FROM_PLAYLIST:     return u"removed_songs_from_playlist"_s;
        case Response::REPEAT_MODE:                     return u"repeat_mode"_s;
        case Response::RESPONSE:                        return u"response"_s;
        case Response::RUNNING_COMMAND:                 return u"running_command"_s;
        case Response::SENT_ALBUM_COVER:                return u"sent_album_cover"_s;
        case Response::SEND_REQUESTED_PLAYLIST:         return u"send_requested_playlist"_s;
        case Response::SET_CURRENT_PLAYLIST_TO:         return u"set_current_playlist_to"_s;
        case Response::SHUFFLE_MODE:                    return u"shuffle_mode"_s;
        case Response::SHUFFLED_PLAYLIST:               return u"shuffled_playlist"_s;
        case Response::SHUFFLED_ALL_PLAYLISTS:          return u"shuffled_all_playlists"_s;
    }
    Q_UNREACHABLE();
}

QString toString(RadioData value){
    switch (value) {
        case RadioData::BITRATE:        return u"bitrate"_s;
        case RadioData::CHANNELS:       return u"channels"_s;
        case RadioData::CLICK_COUNT:    return u"clickcount"_s;
        case RadioData::CHAN_ID:        return u"chan_id"_s;
        case RadioData::CHAN_NAME:      return u"chan_name"_s;
        case RadioData::CODEC:          return u"codec"_s;
        case RadioData::COUNTRY:        return u"country"_s;
        case RadioData::DESCRIPTION:    return u"description"_s;
        case RadioData::DONATE:         return u"donate"_s;
        case RadioData::FORMAT:         return u"format"_s;
        case RadioData::GENRE:          return u"genre"_s;
        case RadioData::HOMEPAGE:       return u"homepage"_s;
        case RadioData::HTTP:           return u"http"_s;
        case RadioData::IMAGE:          return u"image"_s;
        case RadioData::LANGUAGE:       return u"language"_s;
        case RadioData::LABEL:          return u"label"_s;
        case RadioData::QUALITY:        return u"quality"_s;
        case RadioData::SOURCE:         return u"source"_s;
        case RadioData::STREAM_NAME:    return u"stream_name"_s;
        case RadioData::STATION_NAME:   return u"station_name"_s;
        case RadioData::STATION_LIST:   return u"station_list"_s;
        case RadioData::STATION_SOURCE: return u"station_source"_s;
        case RadioData::STATION_UUID:   return u"stationuuid"_s;
        case RadioData::STREAMS:        return u"streams"_s;
        case RadioData::STREAM_ID:      return u"stream_id"_s;
        case RadioData::STREAM_URL:     return u"stream_url"_s;
        case RadioData::STATION_URL:    return u"station_url"_s;
        case RadioData::TAGS:           return u"tags"_s;
        case RadioData::URL:            return u"url"_s;
        case RadioData::VOTES:          return u"votes"_s;
    }
    Q_UNREACHABLE();
}

QString toString(RadioSource value){
    switch (value) {
        case RadioSource::RADIOBROWSER:         return u"radioBrowser"_s;
        case RadioSource::RADIOBROWSERPRETTY:   return u"Radio Browser"_s;
        case RadioSource::RADIOPARADISE:        return u"radioParadise"_s;
        case RadioSource::RADIOPARADISEPRETTY:  return u"Radio Paradise"_s;
        case RadioSource::QOBUZ:                return u"qobuz"_s;
        case RadioSource::QOBOZPRETTY:          return u"Qobuz"_s;
        case RadioSource::SOMAFM:               return u"somaFm"_s;
        case RadioSource::SOMAFMPRETTY:         return u"Soma FM"_s;
        case RadioSource::SPOTIFY:              return u"spotify"_s;
        case RadioSource::SPOTIFYPRETTY:        return u"Spotify"_s;
        case RadioSource::SUBSONIC:             return u"subsonic"_s;
        case RadioSource::SUBSONICPRETTY:       return u"Subsonic"_s;
        case RadioSource::TIDAL:                return u"tidal"_s;
        case RadioSource::TIDALPRETTY:          return u"Tidal"_s;
        case RadioSource::UNKNOWN:              return u"unknown"_s;
    }
    Q_UNREACHABLE();
}

} // namespace RemoteTypes
