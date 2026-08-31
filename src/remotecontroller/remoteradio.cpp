#include <QJsonArray>
#include <QJsonParseError>

#include "radios/radiobackend.h"
#include "radios/radioservices.h"
#include "covermanager/currentalbumcoverloader.h"

#include "remoteradio.h"
#include "remotejsoncreator.h"
#include "remotetypes.h"

using namespace RemoteTypes;

RemoteRadio::RemoteRadio(const Application *app, QObject *parent)
    :QObject{parent},
    app_{app}
{
    QObject::connect(&*app_->radio_services()->radio_backend(), &RadioBackend::NewChannels, this, &RemoteRadio::gotChannels);
    QObject::connect(&*app_->current_albumcover_loader(), &CurrentAlbumCoverLoader::AlbumCoverLoaded, this, &RemoteRadio::getImage);
    QObject::connect(&*app_->radio_services(), &RadioServices::OnRawDataReceived, this, &RemoteRadio::RawDataReceived);
    // QObject::connect(&*app_->radio_services(), &RadioServices::RadioBrowserSearchFinished, this, &RemoteRadio::RadioBrowserSearchFinished);
    app_->radio_services()->RefreshChannels(); // delete when done testing radio stations.
}

void RemoteRadio::gotChannels(const RadioChannelList &channels){

    QJsonObject radioStations;
    QJsonArray  radioArray;

    for(auto& chan: channels){
        QJsonObject station;

        station[toString(Arguments::ID)] = chan.ToSong().title();
        station[toString(Arguments::NAME)] = chan.name;
        station[toString(Arguments::STATION_SOURCE)] = chan.ToSong().DescriptionForSource();
        station[toString(Arguments::IMAGE)] = chan.thumbnail_url.toString();
        station[toString(Arguments::STATION_URL)] = chan.ToSong().url().toString();

        // Split on both comma and space, remove empty/duplicate entries
        QStringList tags = chan.tags.split(QRegularExpression(u"[,\\s]+"_s), Qt::SkipEmptyParts);
        // Remove duplicates
        tags.removeDuplicates();
        station[toString(Arguments::TAGS)] = QJsonArray::fromStringList(tags);
        radioArray.push_back(station);
    }
    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::RADIO_STATIONS)),
        field(Arguments::STATION_LIST, radioArray),
        field(Arguments::STATION_SOURCE, QString())
    }));
}

void RemoteRadio::getImage(const Song &song, const AlbumCoverLoaderResult &result){
    QString title = song.title().startsWith(u"http"_s) ? song.artist() : song.title();

    QByteArray imageData;

    if (!result.album_cover.image_data.isEmpty()) {
        imageData = result.album_cover.image_data;
    } else if (result.temp_cover_url.isValid()) {
        QFile file(result.temp_cover_url.toLocalFile());
        if (file.open(QIODevice::ReadOnly)) {
            imageData = file.readAll();
            file.close();
        }
    }

    if (imageData.isEmpty()) return;

    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::COVER_IMAGE)),
        field(Arguments::NAME, title),
        field(Arguments::COVER_IMAGE, QString::fromLatin1(imageData.toBase64()))
    }));
}

void RemoteRadio::RadioBrowserSearchFinished(const RadioChannelList &channels, const bool has_more){
    Q_UNUSED(has_more);
    gotChannels(channels);
    for (auto& chan: channels){
        qInfo() << "Radiobrowser channel: " << chan.name << " country: " << chan.country \
                << " tags: " << chan.tags << " url: " << chan.url << " id: " << chan.ToSong().title()\
                << " station source: " << chan.ToSong().DescriptionForSource() << " station url: " \
                <<  chan.ToSong().url().toString()
            ;
    }
}

void RemoteRadio::radioBrowserParse(const QJsonArray& data, const QString radioStation){

    if (data.isEmpty()) return;

    QJsonArray station_array;

    for ( auto& chan: data){
        const QJsonObject chan_obj{chan.toObject()};
        if (!chan_obj.contains(u"bitrate"_s)) break;

        QJsonObject station;

        station[toString(Arguments::BITRATE)] = chan_obj[u"bitrate"_s].toInt(0);
        station[toString(Arguments::CLICK_COUNTS)] = chan_obj[u"clickcount"_s].toInt(0);
        station[toString(Arguments::COUNTRY)] = chan_obj[u"country"_s].toString(u""_s);
        station[toString(Arguments::DESCRIPTION)] = QString();
        station[toString(Arguments::DONATE)] = QString();
        station[toString(Arguments::FORMAT)] = chan_obj[u"codec"_s].toString(u""_s);
        station[toString(Arguments::HOMEPAGE)] = chan_obj[u"homepage"].toString(u""_s);
        station[toString(Arguments::ID)] = chan_obj[u"stationuuid"_s].toString(u""_s);
        station[toString(Arguments::IMAGE)] = QString();
        station[toString(Arguments::NAME)] = chan_obj[u"name"].toString(u""_s);
        station[toString(Arguments::STATION_URL)] = chan_obj[u"url"_s].toString(u""_s);

        const QStringList tags = chan_obj[u"tags"_s].toString().split(u',', Qt::SkipEmptyParts);
        const QJsonArray tagsArray = QJsonArray::fromStringList(tags);
        station[toString(Arguments::TAGS)] = tagsArray;
        station[toString(Arguments::VOTES)] = chan_obj[u"votes"_s].toInt(0);

        station_array.append(station);
    }

    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::RADIO_STATIONS)),
        field(Arguments::STATION_SOURCE, radioStation),
        field(Arguments::STATION_LIST, station_array)
    }));
}

void RemoteRadio::radioParadiseParse(const QJsonObject& data, const QString radioStation){

    const QJsonArray channels = data[u"channels"_s].toArray();
    QJsonArray station_array;

    for (auto chan : channels){
        const QJsonObject chan_obj{chan.toObject()};
        QJsonObject station;

        station[toString(Arguments::BITRATE)] = 0;
        station[toString(Arguments::CLICK_COUNTS)] = 0;
        station[toString(Arguments::COUNTRY)] = QString();
        station[toString(Arguments::DESCRIPTION)] = chan_obj[u"chan_name"_s].toString(u""_s);
        station[toString(Arguments::DONATE)] = QString();
        station[toString(Arguments::FORMAT)] = QString();
        station[toString(Arguments::GENRE)] = QString();
        station[toString(Arguments::HOMEPAGE)] = QString();
        station[toString(Arguments::ID)] = QString::number(chan_obj[u"chan_id"_s].toInt(0));
        station[toString(Arguments::IMAGE)] = chan_obj[u"image"_s].toString(u""_s);
        station[toString(Arguments::NAME)] = chan_obj[u"chan_name"_s].toString(u""_s);
        station[toString(Arguments::STATION_URL)] = QString();
        station[toString(Arguments::TAGS)] = QJsonArray();
        station[toString(Arguments::VOTES)] = 0;

        // Add playlists to the station.
        const QJsonArray playlists = chan_obj[u"streams"_s].toArray();
        QJsonArray playlist_array;

        for (auto playlist : playlists){
            QJsonObject playlist_obj = playlist.toObject();
            QJsonObject playlist_data;
            playlist_data[toString(Arguments::STREAM_URL)] = playlist_obj[u"url"_s].toString(u""_s);
            playlist_data[toString(Arguments::FORMAT)] = playlist_obj[u"label"_s].toString(u""_s);
            playlist_data[toString(Arguments::QUALITY)] = playlist_obj[u"stream_id"_s].toString(u""_s);
            playlist_array.append(playlist_data);
        }
        station[u"playlists"_s] = playlist_array;
        station_array.append(station);
    }

    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::RADIO_STATIONS)),
        field(Arguments::STATION_SOURCE, radioStation),
        field(Arguments::STATION_LIST, station_array)
    }));
}

void RemoteRadio::RawDataReceived(const QByteArray &data, Song::Source source){

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) return;

    if (doc.isArray() && source == Song::Source::RadioBrowser) {
        radioBrowserParse(doc.array(),  u"radioBrowser"_s);
    }

    QJsonObject root = doc.object();

    switch(source){
        case Song::Source::SomaFM: {
            somaFmParse(root , u"somaFM"_s);
            break;
        }
        case Song::Source::RadioParadise: {
            radioParadiseParse(root, u"radioParadise"_s);
            break;
        }
        case Song::Source::RadioBrowser: {
            qInfo() << "rawdata called radio browser switch";
            // radioBrowserParse(root, u"radioBrowser"_s);
            break;
        }
        case Song::Source::Spotify: {
            break;
        }
        case Song::Source::Subsonic: {
            break;
        }
        case Song::Source::Tidal: {
            break;
        }
        case Song::Source::Qobuz: {
            break;
        }
        default: break;
    }
}

void RemoteRadio::somaFmParse(const QJsonObject& data, const QString radioStation){

    const QJsonArray channels = data[u"channels"_s].toArray();
    QJsonArray station_array;

    for (auto chan : channels){
        const QJsonObject chan_obj{chan.toObject()};
        QJsonObject station;
        
        station[toString(Arguments::BITRATE)] = 0;
        station[toString(Arguments::CLICK_COUNTS)] = 0;
        station[toString(Arguments::COUNTRY)] = QString();
        station[toString(Arguments::DESCRIPTION)] = chan_obj[u"description"_s].toString(u""_s);
        station[toString(Arguments::DONATE)] = QString();
        station[toString(Arguments::FORMAT)] = QString();
        station[toString(Arguments::GENRE)] = chan_obj[u"genre"_s].toString(u""_s);
        station[toString(Arguments::HOMEPAGE)] = QString();
        station[toString(Arguments::ID)] = chan_obj[u"id"_s].toString(u""_s); // QString::number(chan_obj[u"id"_s].toInt(0));
        station[toString(Arguments::IMAGE)] = chan_obj[u"image"_s].toString(u""_s);
        station[toString(Arguments::NAME)] = chan_obj[u"title"_s].toString(u""_s);
        station[toString(Arguments::STATION_URL)] = QString();
        station[toString(Arguments::TAGS)] = QJsonArray();
        station[toString(Arguments::VOTES)] = 0;


        // Add playlists to the station.
        const QJsonArray playlists = chan_obj[u"playlists"_s].toArray();
        QJsonArray playlist_array;

        for (auto playlist : playlists){
            QJsonObject playlist_obj = playlist.toObject();
            QJsonObject playlist_data;

            playlist_data[toString(Arguments::STREAM_URL)] = playlist_obj[u"url"_s].toString(u""_s);
            playlist_data[toString(Arguments::FORMAT)] = playlist_obj[u"format"_s].toString(u""_s);
            playlist_data[toString(Arguments::QUALITY)] = playlist_obj[u"quality"_s].toString(u""_s);
            playlist_array.append(playlist_data);
        }
        station[u"playlists"_s] = playlist_array;
        station_array.append(station);
    }

    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::RADIO_STATIONS)),
        field(Arguments::STATION_SOURCE, radioStation),
        field(Arguments::STATION_LIST, station_array)
    }));
}

