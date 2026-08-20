#include "remoteradio.h"

#include "collection/collectionview.h"
RemoteRadio::RemoteRadio(const Application *app, QObject *parent)
    :QObject{parent},
    app_{app},
    collectionview_{}
{
    QObject::connect(&*app_->radio_services()->radio_backend(), &RadioBackend::NewChannels, this, &RemoteRadio::GotChannelsFromBackend);
    auto test = app_->radio_services()->radio_backend();
    test->GetChannelsAsync();
    collectionview_->TotalSongs();
}
