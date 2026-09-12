#ifndef REMOTETYPES_H
#define REMOTETYPES_H

#include <QString>
#include <QJsonValue>

using namespace Qt::Literals::StringLiterals;

namespace RemoteTypes{

    enum class Arguments{
        // ACTIVE_PLAYLIST,
        ALBUM,
        ARGUMENT,
        ARGUMENTS,
        ARTIST,
        COMMAND,
        COVER_ART,
        COVER_IMAGE,
        CURRENT_SONG,
        EMPTY,
        EMPTY_FIELDS,
        ERROR,
        ID,
        NAME,
        NONCE,
        PAUSED,
        PLAYING,
        PROOF,
        REQUIRED,
        STOPPED,
        TIME,
        TITLE
    };

    enum class Auth{
        AUTH,
        AUTH_FAILED,
        AUTH_SUCCESS,
        CHALLENGE,
        ERROR
    };

    enum class Error{
        COMMAND_NOT_FOUND,
        COVER_NOT_FOUND,
        ERROR,
        PLAYLIST_NOT_CLEARED,
        PLAYLIST_NOT_CLOSED,
        PLAYLIST_NOT_FOUND,
        WRONG_ARGUMENT_SENT,
        WRONG_NUMBER_ARGUMENTS_PASSED,
    };

    enum class Event{
        CLOSED_PLAYLIST_WITH_ID,
        EVENT,
        GUI_UPDATES,
        MAKE_ALL_PLAYLISTS,
        MAKE_PLAYLIST,
        MUSIC_TOTALS,
        MUTE,
        NEXT,
        NEW_PLAYLIST,
        PAUSE,
        PLAY,
        PLAY_PAUSE,
        PREVIOUS,
        RADIO_STATIONS,
        RESTART_OR_PREVIOUS,
        RENAME_PLAYLIST,
        REPEAT_MODE,
        SEEK_BACKWARD,
        SEEK_FORWARD,
        SEEK_TO,
        SENT_ACTIVE_PLAYLIST,
        SHUFFLE_MODE,
        SONG_CHANGED,
        SONG_INFO,
        STOP,
        STOP_AFTER_CURRENT,
        VOLUME_CHANGED,
        VOLUME,
        VOLUME_DOWN,
        VOLUME_UP
    };

    enum class MessageType{
        AUTH,
        ERROR,
        EVENT,
        RESPONSE
    };

    enum class PlaylistCommandMap{
        CLEAR_PLAYLIST,
        CLOSE_PLAYLIST,
        DELETE_PLAYLIST,
        FAVOURITE_PLAYLIST,
        REMOTE_CHANGED_PLAYLIST,
        REMOTE_SENT_ACTIVE,
        REMOVE_DUPLICATES_PLAYLIST,
        REMOVE_UNAVAILABLE_SONGS,
        REMOVE_SONGS_PLAYLIST,
        RENAME_PLAYLIST,
        REPEAT_MODE,
        SEND_ACTIVE_PLAYLIST_SONG,
        SEND_ALL_PLAYLISTS,
        SEND_PLAYLIST,
        REQUEST_COVER,
        SET_CURRENT_PLAYLIST,
        SHUFFLE_CURRENT_PLAYLIST,
        SHUFFLE_MODE
    };

    enum class PlaylistData{
        ACTIVE_PLAYLIST,
        ALL,
        CURRENT_PLAYLIST,
        FAVOURITE,
        FROM_INDEX,
        GROUPING,
        IS_FAVOURITE,
        LENGTH,
        OFF,
        PLAYLIST,
        PLAYLIST_ID,
        PLAYLISTS,
        PLAYLIST_LENGTH,
        PLAYLIST_SIZE,
        POSITION,
        REPEAT_INTRO,
        REPEAT_MODE,
        REPEAT_PLAYLIST,
        REPEAT_ONEBYONE,
        REPEAT_TRACK,
        ROW,
        SHUFFLE_MODE,
        SONG_INDEX,
        SONGS,
        SONGS_LIST,
        TO_INDEX,
        TOTAL_ALBUMS,
        TOTAL_ARTISTS,
        TOTAL_SONGS,
        TRACK,
        TRACK_ID,
        SONG_URL,
    };

    enum class Response{
        CLEARED_PLAYLIST,
        CURRENT_SONG,
        DELETED_PLAYLIST_WITH_ID,
        IS_PLAYLIST_A_FAVOURITE,
        PLAYLIST_CHANGED,
        PLAYLIST_CLOSED,
        REMOVED_DUPLICATES_FROM_PLAYLIST,
        REMOVED_UNAVAILABLE_SONGS,
        RENAME_PLAYLIST,
        REMOVED_SONGS_FROM_PLAYLIST,
        REPEAT_MODE,
        RESPONSE,
        RUNNING_COMMAND,
        SENT_ALBUM_COVER,
        SEND_REQUESTED_PLAYLIST,
        SET_CURRENT_PLAYLIST_TO,
        SHUFFLE_MODE,
        SHUFFLED_PLAYLIST,
        SHUFFLED_ALL_PLAYLISTS
    };

    enum class RadioData{
          BITRATE,
          CHANNELS,
          CLICK_COUNT,
          CHAN_ID,
          CHAN_NAME,
          CODEC,
          COUNTRY,
          DESCRIPTION,
          DONATE,
          FAVICON,
          FORMAT,
          GENRE,
          HOMEPAGE,
          HTTP,
          IMAGE,
          LABEL,
          LANGUAGE,
          QUALITY,
          SOURCE,
          SOURCE_LOGO,
          STATION_LIST,
          STATION_NAME,
          STATION_SOURCE,
          STATION_URL,
          STATION_UUID,
          STREAMS,
          STREAM_ID,
          STREAM_NAME,
          STREAM_URL,
          TAGS,
          URL,
          VOTES
    };

    enum class RadioSource{
        RADIOBROWSER,
        RADIOBROWSERPRETTY,
        RADIOPARADISE,
        RADIOPARADISEPRETTY,
        SOMAFM,
        SOMAFMPRETTY,
        QOBUZ,
        QOBOZPRETTY,
        SPOTIFY,
        SPOTIFYPRETTY,
        SUBSONIC,
        SUBSONICPRETTY,
        TIDAL,
        TIDALPRETTY,
        UNKNOWN
    };


      QString toString(Arguments value);
      QString toString(Auth auth);
      QString toString(Event event);
      QString toString(Error error);
      QString toString(MessageType type);
      QString toString(PlaylistCommandMap data);
      QString toString(PlaylistData data);
      QString toString(RadioData data);
      QString toString(RadioSource source);
      QString toString(Response response);

      template<typename T>
      std::pair<QString, QJsonValue> field(T key, QJsonValue value) {
          return {toString(key), value};
      }

      // Specialisation for MessageType
      template<>
      inline std::pair<QString, QJsonValue> field(MessageType key, QJsonValue value) {
          Q_UNUSED(value); // value is derived from the key itself
          return {u"type"_s, toString(key)};
      }
}
#endif // REMOTETYPES_H



