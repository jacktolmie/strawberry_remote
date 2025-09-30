#ifndef REMOTEGUIVALUES_H
#define REMOTEGUIVALUES_H

#include <QObject>
#include <QJsonObject>
#include <QTcpSocket>

// #include "core/song.h"
#include "core/application.h"
#include "ui_mainwindow.h"

class TrackSlider;

class RemoteGuiValues : public QObject
{
  Q_OBJECT

  const Application* app_;
  const Ui_MainWindow *ui_;

  void getUpdates(QTcpSocket *client);

public:
  explicit RemoteGuiValues(const Application* app, QObject *parent);

public Q_SLOTS:
  void triggerUpdate(QTcpSocket *client);

Q_SIGNALS:
  void sendCurrentStatus(QTcpSocket *client, QJsonObject data);

};

#endif // REMOTEGUIVALUES_H
