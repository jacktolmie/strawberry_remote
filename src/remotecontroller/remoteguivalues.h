#ifndef REMOTEGUIVALUES_H
#define REMOTEGUIVALUES_H

#include <QObject>
#include <QJsonObject>
#include <QTcpSocket>

#include "core/application.h"
#include "ui_mainwindow.h"
#include "remotecontroller/remoteplaylist.h"
// #include "remotecontroller/remotecommands.h"

class TrackSlider;

class RemoteGuiValues : public QObject
{
  Q_OBJECT

  const Application     *app_;
  const Ui_MainWindow   *ui_;
  const RemotePlaylist  *remotePlaylist_;

  QJsonObject getUpdates() const;

public:
  explicit RemoteGuiValues(const RemotePlaylist* remotePlaylist, const Application* app, QObject *parent);

public Q_SLOTS:
  QJsonObject triggerUpdate() const;

Q_SIGNALS:
  void sendCurrentStatus(const QJsonObject& response);

};

#endif // REMOTEGUIVALUES_H
