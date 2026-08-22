#include <QJsonArray>

#include "radios/radiobackend.h"
// #include "radios/radioview.h"

#include "remoteradio.h"
#include "remotejsoncreator.h"
#include "remotetypes.h"

using namespace RemoteTypes;

RemoteRadio::RemoteRadio(const Application *app, QObject *parent)
    :QObject{parent},
    app_{app},
    collectionview_{}
{
    QObject::connect(&*app_->radio_services()->radio_backend(), &RadioBackend::NewChannels, this, &RemoteRadio::GotChannelsFromBackend);
    QObject::connect(&*app_->radio_services()->radio_backend(), &RadioBackend::NewChannels, this, &RemoteRadio::GotChannelsFromService);

    app_->radio_services()->radio_backend()->GetChannelsAsync();

}

void RemoteRadio::GotChannelsFromBackend([ [maybe_unused ]]const RadioChannelList &channels){
    qInfo()<< "Radio channels backend called:";
    //     for(auto& chan: channels){
    //     qInfo() << "Station: " << chan.url.toString() << " and name: "<< chan.name;
    // }
}

void RemoteRadio::GotChannelsFromService(const RadioChannelList &channels){
    qInfo()<< "Radio channels service called:";

    QJsonArray radioArray;

    QJsonObject radioStations;

    for(auto& chan: channels){
        QJsonObject station;
        station[toString(Arguments::NAME)] = chan.name;
        station[toString(Arguments::STATION_SOURCE)] = chan.ToSong().DescriptionForSource();
        station[toString(Arguments::COVER_IMAGE)] = chan.thumbnail_url.toString();
        station[toString(Arguments::STATION_URL)] = chan.ToSong().url().toString();

        radioArray.push_back(station);
 }
    for(auto& chan: std::as_const(radioArray)) qInfo() << "blay" << chan;
    Q_EMIT sendResponse(RemoteJsonCreator::createResponse({
        field(MessageType::EVENT, toString(MessageType::EVENT)),
        field(Event::EVENT, toString(Event::RADIO_STATIONS)),
        field(Arguments::STATION_LIST, radioArray)
    }));
}
