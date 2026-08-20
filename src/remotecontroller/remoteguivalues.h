#ifndef REMOTEGUIVALUES_H
#define REMOTEGUIVALUES_H

#include <QObject>
#include <QJsonObject>
#include <QTcpSocket>

#include "core/application.h"
#include "remoteplaylist.h"

class TrackSlider;

class RemoteGuiValues : public QObject
{
  Q_OBJECT

  const Application     *app_;
  const RemotePlaylist  *remotePlaylist_;

  QJsonObject getUpdates() const;

public:
  QJsonObject triggerUpdate() const;

public:
  explicit RemoteGuiValues(const RemotePlaylist* remotePlaylist, const Application* app, QObject *parent);

Q_SIGNALS:
  void sendCurrentStatus(const QJsonObject& response);

private Q_SLOTS:
   void musicTotals(const int count);

};

#endif // REMOTEGUIVALUES_H
