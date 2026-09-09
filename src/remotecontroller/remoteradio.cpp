#include <QJsonArray>
#include <QJsonParseError>

#include "radios/radiobackend.h"
#include "radios/radioparadiseservice.h"
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
    QObject::connect(&*app_->current_albumcover_loader(), &CurrentAlbumCoverLoader::AlbumCoverLoaded, this, &RemoteRadio::getImage);
    QObject::connect(&*app_->radio_services(), &RadioServices::OnRawDataReceived, this, &RemoteRadio::RawDataReceived);

    // QObject::connect(&*app_->radio_services()->radio_backend(), &RadioBackend::NewChannels, this, &RemoteRadio::gotChannels);
    // QObject::connect(&*app_->radio_services(), &RadioServices::RadioBrowserSearchFinished, this, &RemoteRadio::RadioBrowserSearchFinished);
    app_->radio_services()->RefreshChannels(); // delete when done testing radio stations.
}

void RemoteRadio::getStationsFromClient(const QJsonObject& args){

    QString sourceString{args[toString(RadioData::SOURCE)].toString(toString(Arguments::EMPTY))};
    if (sourceString.isEmpty()) return;

    RadioChannelList channels{};

    QString source = args[toString(RadioData::SOURCE)].toString();
    QJsonArray streams = args[toString(RadioData::STREAMS)].toArray();
    for (const auto& stream : std::as_const(streams)) {
        QJsonObject streamObj = stream.toObject();
        QString name = streamObj[toString(Arguments::NAME)].toString();
        QUrl url = QUrl(streamObj[toString(RadioData::URL)].toString());

        channels.append(RadioChannel(getSourceFromName(name), name, url));
    }

    app_->radio_services()->radio_backend()->AddChannelsAsync(channels);
}

void RemoteRadio::gotChannels(const RadioChannelList &channels){

    QJsonObject radioStations;
    QJsonArray  radioArray;

    for(auto& chan: channels){
        QJsonObject station;
        qInfo() << "Radiochanneloutput: codec: " << chan.codec << " country: " << chan.country << "url: " << chan.url << " tags: " << chan.tags \
                << " name: " << chan.name << " thumbnail: " << chan.thumbnail_url << " domain: " << chan.ToSong().DomainForSource() \
                << " title: " << chan.ToSong().PrettyTitle() << " bitrate: " << chan.ToSong().bitrate() << " genre: " << chan.ToSong().genre() \
                << " id: " <<chan.ToSong().id() << " song id: " << chan.ToSong().song_id() << " other url: " <<chan.ToSong().url();

        station[toString(Arguments::ID)] = chan.ToSong().title();
        station[toString(Arguments::NAME)] = chan.name;
        station[toString(RadioData::STATION_SOURCE)] = chan.ToSong().DescriptionForSource();
        station[toString(RadioData::IMAGE)] = chan.thumbnail_url.toString();
        station[toString(RadioData::STATION_URL)] = chan.ToSong().url().toString();

        // Split on both comma and space, remove empty/duplicate entries
        QStringList tags = chan.tags.split(QRegularExpression(u"[,\\s]+"_s), Qt::SkipEmptyParts);
        // Remove duplicates
        tags.removeDuplicates();
        station[toString(RadioData::TAGS)] = QJsonArray::fromStringList(tags);
        radioArray.push_back(station);
    }
    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::RADIO_STATIONS)),
        field(RadioData::STATION_LIST, radioArray),
        field(RadioData::STATION_SOURCE, QString())
    }));
}

void RemoteRadio::getImage(const Song &song, const AlbumCoverLoaderResult &result){
    QString title = song.title().startsWith(toString(RadioData::HTTP)) ? song.artist() : song.title();

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
        field(Event::EVENT, toString(Arguments::COVER_IMAGE)),
        field(Arguments::NAME, title),
        field(Arguments::COVER_IMAGE, QString::fromLatin1(imageData.toBase64()))
    }));
}

Song::Source RemoteRadio::getSourceFromName(const QString& name){
    static const QHash<QString, Song::Source> sourceMap = {
        { toString(RadioSource::SOMAFM), Song::Source::SomaFM },
        { toString(RadioSource::RADIOPARADISE), Song::Source::RadioParadise },
        { toString(RadioSource::RADIOBROWSER), Song::Source::RadioBrowser },
        { toString(RadioSource::SPOTIFY), Song::Source::Spotify },
        { toString(RadioSource::SUBSONIC), Song::Source::Subsonic },
        { toString(RadioSource::TIDAL), Song::Source::Tidal },
        { toString(RadioSource::QOBUZ), Song::Source::Qobuz }
    };

    return sourceMap.value(name.toLower(), Song::Source::Unknown);
}

QString RemoteRadio::getNameFromSource(Song::Source source){

    switch(source){
        case Song::Source::SomaFM: {
            return toString(RadioSource::SOMAFM);
        }
        case Song::Source::RadioParadise: {
            return toString(RadioSource::RADIOPARADISE);
        }
        case Song::Source::RadioBrowser: {
            return toString(RadioSource::RADIOBROWSER);
        }
        case Song::Source::Spotify: {
            return toString(RadioSource::SPOTIFY);
        }
        case Song::Source::Subsonic: {
            return toString(RadioSource::SUBSONIC);
        }
        case Song::Source::Tidal: {
            return toString(RadioSource::TIDAL);
        }
        case Song::Source::Qobuz: {
            return toString(RadioSource::QOBUZ);
        }
        default: break;
    }

    return toString(RadioSource::UNKNOWN);
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

void RemoteRadio::radioBrowserParse(const QJsonArray& data, const QString radioStation, RadioService* service){

    if (data.isEmpty()) return;

    QJsonArray station_array;

    for ( auto& chan: data){
        const QJsonObject chan_obj{chan.toObject()};
        if (!chan_obj.contains(toString(RadioData::BITRATE))) break;

        QJsonObject station;

        station[toString(RadioData::BITRATE)] = chan_obj[toString(RadioData::BITRATE)].toInt(0);
        station[toString(RadioData::CLICK_COUNT)] = chan_obj[toString(RadioData::CLICK_COUNT)].toInt(0);
        station[toString(RadioData::COUNTRY)] = chan_obj[toString(RadioData::COUNTRY)].toString(toString(Arguments::EMPTY));
        station[toString(RadioData::DESCRIPTION)] = QString();
        station[toString(RadioData::DONATE)] = service ? service->Donate().toString() : toString(Arguments::EMPTY);
        station[toString(RadioData::FORMAT)] = chan_obj[toString(RadioData::CODEC)].toString(toString(Arguments::EMPTY));

        const QStringList genres = chan_obj[toString(RadioData::TAGS)].toString().split(u',', Qt::SkipEmptyParts);
        const QJsonArray genressArray = QJsonArray::fromStringList(genres);
        station[toString(RadioData::GENRE)] = genressArray;
        station[toString(RadioData::HOMEPAGE)] = chan_obj[toString(RadioData::HOMEPAGE)].toString(toString(Arguments::EMPTY));
        station[toString(Arguments::ID)] = chan_obj[toString(RadioData::STATION_UUID)].toString(toString(Arguments::EMPTY));
        station[toString(RadioData::IMAGE)] = QString();
        station[toString(RadioData::LANGUAGE)] = chan_obj[toString(RadioData::LANGUAGE)].toString(toString(Arguments::EMPTY));
        station[toString(RadioData::STATION_NAME)] = chan_obj[toString(Arguments::NAME)].toString(toString(Arguments::EMPTY));
        station[toString(RadioData::STREAM_NAME)] = chan_obj[toString(Arguments::NAME)].toString(toString(Arguments::EMPTY));
        station[toString(PlaylistData::PLAYLISTS)] = QJsonArray();
        station[toString(RadioData::STATION_SOURCE)] = radioStation;
        station[toString(RadioData::STATION_URL)] = chan_obj[toString(RadioData::URL)].toString(toString(Arguments::EMPTY));
        station[toString(RadioData::VOTES)] = chan_obj[toString(RadioData::VOTES)].toInt(0);

        station_array.append(station);
    }

    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::RADIO_STATIONS)),
        field(RadioData::STATION_NAME, toString(RadioSource::RADIOBROWSERPRETTY)),
        field(RadioData::STATION_SOURCE, radioStation),
        field(RadioData::STATION_LIST, station_array)
    }));
}

void RemoteRadio::radioParadiseParse(const QJsonObject& data, const QString radioStation, RadioService *service){

    const QJsonArray channels = data[toString(RadioData::CHANNELS)].toArray();
    QJsonArray station_array;

    for (auto chan : channels){
        const QJsonObject chan_obj{chan.toObject()};
        QJsonObject station;

        station[toString(RadioData::BITRATE)] = 0;
        station[toString(RadioData::CLICK_COUNT)] = 0;
        station[toString(RadioData::COUNTRY)] = QString();
        station[toString(RadioData::DESCRIPTION)] = chan_obj[toString(RadioData::CHAN_NAME)].toString(toString(Arguments::EMPTY));
        station[toString(RadioData::DONATE)] = service ? service->Donate().toString() : toString(Arguments::EMPTY);;
        station[toString(RadioData::FORMAT)] = QString();
        station[toString(RadioData::GENRE)] = QJsonArray();
        station[toString(RadioData::HOMEPAGE)] = service ? service->Homepage().toString() : toString(Arguments::EMPTY);
        station[toString(Arguments::ID)] = QString::number(chan_obj[toString(RadioData::CHAN_ID)].toInt(0));
        station[toString(RadioData::IMAGE)] = chan_obj[toString(RadioData::IMAGE)].toString(toString(Arguments::EMPTY));
        station[toString(RadioData::LANGUAGE)] = chan_obj[toString(RadioData::LANGUAGE)].toString(toString(Arguments::EMPTY));
        station[toString(RadioData::STREAM_NAME)] = chan_obj[toString(RadioData::CHAN_NAME)].toString(toString(Arguments::EMPTY));
        station[toString(RadioData::STATION_NAME)] = toString(RadioSource::RADIOPARADISEPRETTY);
        station[toString(RadioData::STATION_SOURCE)] = radioStation;
        station[toString(RadioData::STATION_URL)] = service ? service->Homepage().toString() : toString(Arguments::EMPTY);
        station[toString(RadioData::VOTES)] = 0;

        // Add playlists to the station.
        const QJsonArray playlists = chan_obj[toString(RadioData::STREAMS)].toArray();
        QJsonArray playlist_array;

        for (auto playlist : playlists){
            QJsonObject playlist_obj = playlist.toObject();
            QJsonObject playlist_data;
            playlist_data[toString(RadioData::STREAM_URL)] = playlist_obj[toString(RadioData::URL)].toString(toString(Arguments::EMPTY));
            playlist_data[toString(RadioData::FORMAT)] = playlist_obj[toString(RadioData::LABEL)].toString(toString(Arguments::EMPTY));
            playlist_data[toString(RadioData::QUALITY)] = playlist_obj[toString(RadioData::STREAM_ID)].toString(toString(Arguments::EMPTY));
            playlist_array.append(playlist_data);
        }
        station[toString(PlaylistData::PLAYLISTS)] = playlist_array;
        station_array.append(station);
    }

    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::RADIO_STATIONS)),
        field(RadioData::STATION_NAME, toString(RadioSource::RADIOPARADISEPRETTY)),
        field(RadioData::STATION_SOURCE, radioStation),
        field(RadioData::STATION_LIST, station_array)
    }));
}

void RemoteRadio::RawDataReceived(const QByteArray &data, Song::Source source, RadioService* service){

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) return;

    QString sourceString{getNameFromSource(source)};

    if (doc.isArray() && source == Song::Source::RadioBrowser) {
        radioBrowserParse(doc.array(),  sourceString, service);
    }

    QJsonObject root = doc.object();

    switch(source){
        case Song::Source::SomaFM: {
            somaFmParse(root , sourceString, service);
            break;
        }
        case Song::Source::RadioParadise: {
            radioParadiseParse(root,sourceString, service);
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

void RemoteRadio::somaFmParse(const QJsonObject& data, const QString radioStation, RadioService *service){

    const QJsonArray channels = data[toString(RadioData::CHANNELS)].toArray();
    QJsonArray station_array;

    for (auto chan : channels){
        const QJsonObject chan_obj{chan.toObject()};
        QJsonObject station;
        
        station[toString(RadioData::BITRATE)] = 0;
        station[toString(RadioData::CLICK_COUNT)] = 0;
        station[toString(RadioData::COUNTRY)] = QString();
        station[toString(RadioData::DESCRIPTION)] = chan_obj[toString(RadioData::DESCRIPTION)].toString(toString(Arguments::EMPTY));
        station[toString(RadioData::DONATE)] = service ? service->Donate().toString() : toString(Arguments::EMPTY);;
        station[toString(RadioData::FORMAT)] = QString();
        
        const QStringList genre = chan_obj[toString(RadioData::GENRE)].toString().split(u'|', Qt::SkipEmptyParts);
        const QJsonArray genreArray = QJsonArray::fromStringList(genre);
        station[toString(RadioData::GENRE)] = genreArray;
        station[toString(RadioData::HOMEPAGE)] = service ? service->Homepage().toString() : toString(Arguments::EMPTY);
        station[toString(Arguments::ID)] = chan_obj[toString(Arguments::ID)].toString(toString(Arguments::EMPTY));
        station[toString(RadioData::IMAGE)] = chan_obj[toString(RadioData::IMAGE)].toString(toString(Arguments::EMPTY));
        station[toString(RadioData::LANGUAGE)] = chan_obj[toString(RadioData::LANGUAGE)].toString(toString(Arguments::EMPTY));
        station[toString(RadioData::STREAM_NAME)] = chan_obj[toString(Arguments::TITLE)].toString(toString(Arguments::EMPTY));
        station[toString(RadioData::STATION_NAME)] = toString(RadioSource::SOMAFMPRETTY);
        station[toString(RadioData::STATION_SOURCE)] = radioStation;
        station[toString(RadioData::STATION_URL)] = service ? service->Homepage().toString() : toString(Arguments::EMPTY);
        station[toString(RadioData::VOTES)] = 0;


        // Add playlists to the station.
        const QJsonArray playlists = chan_obj[toString(PlaylistData::PLAYLISTS)].toArray();
        QJsonArray playlist_array;

        for (auto playlist : playlists){
            QJsonObject playlist_obj = playlist.toObject();
            QJsonObject playlist_data;

            playlist_data[toString(RadioData::STREAM_URL)] = playlist_obj[toString(RadioData::URL)].toString(toString(Arguments::EMPTY));
            playlist_data[toString(RadioData::FORMAT)] = playlist_obj[toString(RadioData::FORMAT)].toString(toString(Arguments::EMPTY));
            playlist_data[toString(RadioData::QUALITY)] = playlist_obj[toString(RadioData::QUALITY)].toString(toString(Arguments::EMPTY));
            playlist_array.append(playlist_data);
        }
        station[toString(PlaylistData::PLAYLISTS)] = playlist_array;
        station_array.append(station);
    }

    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::RADIO_STATIONS)),
        field(RadioData::STATION_SOURCE, radioStation),
        field(RadioData::STATION_NAME, toString(RadioSource::SOMAFMPRETTY)),
        field(RadioData::STATION_LIST, station_array)
    }));
}

