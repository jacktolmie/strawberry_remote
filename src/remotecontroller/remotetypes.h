#ifndef REMOTETYPES_H
#define REMOTETYPES_H

#include <QString>
#include <QJsonValue>

using namespace Qt::Literals::StringLiterals;

namespace RemoteTypes{

    enum class Arguments{
        ACTIVE_PLAYLIST,
        ALBUM,
        ARGUMENT,
        ARGUMENTS,
        ARTIST,
        COMMAND,
        COVER_IMAGE,
        CURRENT_PLAYLIST,
        CURRENT_SONG,
        FAVOURITE,
        ID,
        IS_FAVOURITE,
        LENGTH,
        NAME,
        NONCE,
        REPEAT_MODE,
        REQUIRED,
        ROW,
        PAUSED,
        PLAYLIST,
        PLAYLIST_ID,
        PLAYLISTS,
        PLAYLIST_LENGTH,
        PLAYLIST_SIZE,
        PLAYING,
        POSITION,
        SONGS,
        STOPPED,
        TIME,
        TITLE,
        TRACK_ID,
        SONG_URL,
        VOLUME
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
        ACTIVE_PLAYLIST,
        CLOSED_PLAYLIST_WITH_ID,
        COVER_IMAGE,
        DELETE_PLAYLIST,
        EVENT,
        FAVOURITE_PLAYLIST,
        GUI_UPDATES,
        MAKE_ALL_PLAYLISTS,
        MAKE_PLAYLIST,
        NEXT,
        NEW_PLAYLIST,
        PAUSE,
        PLAY,
        PREVIOUS,
        RENAME_PLAYLIST,
        REPEAT_MODE,
        SEEK_BACKWARD,
        SEEK_FORWARD,
        SEEK_TO,
        SENT_ACTIVE_PLAYLIST,
        SONG_CHANGED,
        SONG_INFO,
        STOP,
        VOLUME_CHANGED
      };

      enum class MessageType
      {
        AUTH,
        ERROR,
        EVENT,
        RESPONSE
      };

      enum class Response{
        CLEARED_PLAYLIST,
        DELETED_PLAYLIST_WITH_ID,
        IS_PLAYLIST_A_FAVOURITE,
        PLAYLIST_CLOSED,
        REMOVED_DUPLICATES_FROM_PLAYLIST,
        RENAME_PLAYLIST,
        REMOVED_SONGS_FROM_PLAYLIST,
        REPEAT_MODE,
        RESPONSE,
        RUNNING_COMMAND,
        SENT_ALBUM_COVER,
        SEND_REQUESTED_PLAYLIST,
        SET_CURRENT_PLAYLIST_TO,
        SHUFFLED_PLAYLIST,
        SHUFFLED_ALL_PLAYLISTS
      };


      QString toString(Auth auth);
      QString toString(Event event);
      QString toString(Error error);
      QString toString(Response response);
      QString toString(MessageType type);
      QString toString(Arguments value);

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



