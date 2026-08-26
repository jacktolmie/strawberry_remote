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
    // QObject::connect(&*app_->radio_services()->radio_backend(), &RadioBackend::NewChannels, this, &RemoteRadio::gotChannels);
    QObject::connect(&*app_->current_albumcover_loader(), &CurrentAlbumCoverLoader::AlbumCoverLoaded, this, &RemoteRadio::getImage);
    QObject::connect(&*app_->radio_services(), &RadioServices::OnRawDataReceived, this, &RemoteRadio::RawDataReceived);
    app_->radio_services()->RefreshChannels();
}

void RemoteRadio::gotChannels(const RadioChannelList &channels){

    QJsonObject radioStations;
    QJsonArray  radioArray;

    for(auto& chan: channels){
        QJsonObject station;
        station[toString(Arguments::NAME)] = chan.name;
        station[toString(Arguments::STATION_SOURCE)] = chan.ToSong().DescriptionForSource();
        station[toString(Arguments::COVER_IMAGE)] = chan.thumbnail_url.toString();
        station[toString(Arguments::STATION_URL)] = chan.ToSong().url().toString();
        radioArray.push_back(station);
    }
    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::RADIO_STATIONS)),
        field(Arguments::STATION_LIST, radioArray)
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

void RemoteRadio::radioParadiseParse(const QJsonObject& data, const QString radioStation){

}

void RemoteRadio::RawDataReceived(const QByteArray &data, Song::Source source){
    qInfo()<< "Rawdatareceived: " << (int)source;
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) return;

    const QJsonObject root = doc.object();

    switch(source){
        case Song::Source::SomaFM: {
            somaFmParse(root, u"somaFM"_s);
            break;
        }
        case Song::Source::RadioParadise: {
            radioParadiseParse(root, u"radioParadise"_s);
            break;
        }
        case Song::Source::RadioBrowser: {
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
        station[u"id"_s] = chan_obj[u"id"_s].toString();
        station[u"name"_s] = chan_obj[u"title"_s].toString();
        station[u"image"_s] = chan_obj[u"image"_s].toString();
        station[u"genre"_s] = chan_obj[u"genre"_s].toString();
        station[u"description"_s] = chan_obj[u"description"_s].toString();

        // Add playlists to the station.
        const QJsonArray playlists = chan_obj[u"playlists"_s].toArray();
        QJsonArray playlist_array;

        for (auto playlist : playlists){
            QJsonObject playlist_obj = playlist.toObject();
            QJsonObject playlist_data;
            playlist_data[u"url"_s] = playlist_obj[u"url"_s].toString();
            playlist_data[u"format"_s] = playlist_obj[u"format"_s].toString();
            playlist_data[u"quality"_s] = playlist_obj[u"quality"_s].toString();
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

