#ifndef REMOTERADIO_H
#define REMOTERADIO_H

#include "covermanager/albumcoverimageresult.h"
#include "covermanager/albumcoverloaderresult.h"
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
    // QJsonObject appendToCurrentPlaylist();
    // QJsonObject replaceCurrentPlaylist();
    // QJsonObject createNewPlaylist();
    void somaFmParse(const QJsonObject& data, const QString radioStation, RadioService *service);
    void radioBrowserParse(const QJsonArray& data, const QString radioStation, RadioService *service);
    void radioParadiseParse(const QJsonObject& data, const QString radioStation, RadioService *service);

    void getStationsFromClient(const QJsonObject& args);
    QString getNameFromSource(Song::Source source);
    Song::Source getSourceFromName(const QString& name);

private Q_SLOTS:
    void gotChannels(const RadioChannelList &channels);
    void getImage(const Song &song, const AlbumCoverLoaderResult &result);
    void RawDataReceived(const QByteArray &data, Song::Source source, RadioService* service);
    void RadioBrowserSearchFinished(const RadioChannelList &channels, const bool has_more);


Q_SIGNALS:
    void sendResponse(const QJsonObject& response);
};

#endif // REMOTERADIO_H
