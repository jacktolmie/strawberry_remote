#ifndef REMOTEPLAYLIST_H
#define REMOTEPLAYLIST_H

#include <QObject>
#include <QDebug>
#include <QList>

#include "core/application.h"

class RemotePlaylist : public QObject
{
     Q_OBJECT

  QList<QString> commands{
    QStringLiteral("shuffle-playlist"),
    QStringLiteral("load-playlist"),
    QStringLiteral("clear-playlist"),
    QStringLiteral("delete-playlist"),
    QStringLiteral("get-all-playlists"),
    QStringLiteral("save-playlist"),
    QStringLiteral("smart-playlist"),
    QStringLiteral("rename-playlist"),
    QStringLiteral("delete-playlist")
  };

  QMap<QString, std::function<void(const QStringList&)>> createCommandMap();

public:
    explicit RemotePlaylist(Application *app, QObject *parent = nullptr);
    ~RemotePlaylist() = default;

    void processCommand(const QString& command, const QStringList& args);



// signals:

};

#endif // REMOTEPLAYLIST_H
