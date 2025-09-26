#ifndef REMOTEGUIVALUES_H
#define REMOTEGUIVALUES_H

#include <QObject>
#include <QJsonObject>

#include "core/song.h"
#include "core/application.h"

class TrackSlider;

class RemoteGuiValues : public QObject
{
  Q_OBJECT

  Application* app_;

  void getUpdates();

public:
  // explicit RemoteGuiValues(QObject *parent = nullptr);
  explicit RemoteGuiValues(Application *app, QObject *parent = nullptr);



Q_SIGNALS:
  void sendCurrentStatus(QJsonObject data);

};

#endif // REMOTEGUIVALUES_H
