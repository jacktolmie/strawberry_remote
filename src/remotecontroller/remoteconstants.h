#ifndef REMOTECONSTANTS_H
#define REMOTECONSTANTS_H

#include <QStringList>

// namespace remoteconstants
// {
//   // Lambda to check if args contains an int.
//   inline constexpr auto parseUintArg = [](const QStringList& args, bool& ok)-> quint32 {
//     if (args.isEmpty()){
//       ok = false;
//       return 0;
//     }
//     return args.first().toUInt(&ok);
//   };

//   // Lambda to check if args contains a string.
//   inline constexpr auto parseStringArg = [](const QStringList& args, bool& ok)-> QString {
//     if (args.isEmpty()){
//       ok = false;
//       return QString();
//     }
//     return QString(args.first());
//   };

// }
#endif // REMOTECONSTANTS_H
/*
 * QJsonObject RemotePlaylist::setFavouritePlaylist(const QJsonObject& obj)
{
    qint64 id = obj[u"playlistId"_s].toInteger(-1);
    if (id == -1) return wrongNumArgs(2);

    bool isFavourite = obj[u"favourite"_s].toBool();

    Q_EMIT RemotePlaylist::remoteFavouritePlaylist(id, isFavourite);
    return RemoteJsonCreator::createResponse({
        field(MessageType::RESPONSE, toString(MessageType::RESPONSE)),
        field(Response::RESPONSE, toString(Response::IS_PLAYLIST_A_FAVOURITE)),
        field(Arguments::IS_FAVOURITE, app_->playlist_manager()->playlist(id)->is_favorite())
    });
}*/