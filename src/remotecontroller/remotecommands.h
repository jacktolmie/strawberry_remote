#ifndef REMOTECOMMANDS_H
#define REMOTECOMMANDS_H

#include <QObject>

#include "core/application.h"
#include "remoteradio.h"
#include "remoteplaylist.h"
#include "remotebasiccommands.h"

class QTcpSocket;


class RemoteCommands : public QObject
{
  Q_OBJECT

  const Application   *app_;

  RemoteBasicCommands   *basicCommands_;
  RemoteRadio           *remoteRadio_;
  RemotePlaylist        *remotePlaylist_;


  BasicCmdMap           basicCmdMap_;
  PlaylistCmdMap        playlistCmdMap_;
  RadioCmdMap           radioCmdMap_;

  QList<QMap<QString, std::function<QJsonObject(const QJsonObject&)>>*> commandMaps;

public:
  explicit RemoteCommands(const Application *app, QObject *parent);
  ~RemoteCommands() = default;

  void processCommand(const QString& command, const QJsonObject& args);
  const RemotePlaylist* getRemotePlaylist() const { return remotePlaylist_; }

public Q_SLOTS:
  void processLine(const QString& line);

Q_SIGNALS:
  void forwardToPlayer(const QString& command, const QStringList& args);
  void sendResponse(const QJsonObject& response);
};

#endif // REMOTECOMMANDS_H
