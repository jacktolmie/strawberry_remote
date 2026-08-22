#ifndef REMOTERADIO_H
#define REMOTERADIO_H

#include <QObject>

class CollectionView;

#include "core/application.h"
#include "radios/radioservices.h"

class RemoteRadio : public QObject
{
    Q_OBJECT

    const Application   *app_;

public:
    explicit RemoteRadio(const Application *app, QObject *parent = nullptr);
    ~RemoteRadio() = default;

private:
    CollectionView *collectionview_;

private Q_SLOTS:
    void GotChannelsFromBackend(const RadioChannelList &channels);
    void GotChannelsFromService(const RadioChannelList &channels);

Q_SIGNALS:
    void sendResponse(const QJsonObject& response);
};

#endif // REMOTERADIO_H
