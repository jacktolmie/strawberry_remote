#ifndef REMOTEPLAYLIST_H
#define REMOTEPLAYLIST_H

#include <QObject>
#include <QDebug>
#include <QList>

#include "core/application.h"


class RemotePlaylist : public QObject
{
     Q_OBJECT

  Application *app_;

  void createCommandMap();
  QMap<QString, std::function<void(const QStringList&)>> commandMap;

  // Rename current playlist. Send command rename-playlist <new name>.
  void renamePlaylist(const QStringList& args);

  // Shuffle all playlists.
  void shuffleAllPlaylists();

public:
    explicit RemotePlaylist(Application *app, QObject *parent = nullptr);
    ~RemotePlaylist() = default;

    void processCommand(const QString& command, const QStringList& args);



// signals:

};

#endif // REMOTEPLAYLIST_H
