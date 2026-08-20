#ifndef REMOTERADIO_H
#define REMOTERADIO_H

#include <QObject>

class CollectionView;

#include "core/application.h"
#include "radios/radioservices.h"
#include "radios/radiobackend.h"
#include "remotejsoncreator.h"
#include "remotetypes.h"

class RemoteRadio : public QObject
{
    Q_OBJECT

    const Application   *app_;

public:
    explicit RemoteRadio(const Application *app, QObject *parent = nullptr);
    ~RemoteRadio() = default;

private:
    void GotChannelsFromBackend(const RadioChannelList &channels);

    CollectionView *collectionview_;

// signals:
};

#endif // REMOTERADIO_H
