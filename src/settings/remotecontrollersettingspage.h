/*
 * Strawberry Music Player
 * This file was part of Clementine.
 * Copyright 2012, David Sansome <me@davidsansome.com>
 * Copyright 2018-2021, Jonas Kvinge <jonas@jkvinge.net>
 *
 * Strawberry is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Strawberry is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Strawberry.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef REMOTECONTROLLERSETTINGSPAGE_H
#define REMOTECONTROLLERSETTINGSPAGE_H

#include "config.h"

#include <QList>
#include <QNetworkInterface>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "includes/shared_ptr.h"
#include "settingspage.h"
#include "remotecontroller/remotecontroller.h"

class SettingsDialog;

namespace Ui {
  class RemoteControllerSettingsPage;
}

class RemoteControllerSettingsPage : public SettingsPage {
  Q_OBJECT

public:
  explicit RemoteControllerSettingsPage(SettingsDialog *dialog, QWidget *parent = nullptr);
  ~RemoteControllerSettingsPage();

  virtual void Load() override;
  virtual void Save() override;

Q_SIGNALS:

private:
  Ui::RemoteControllerSettingsPage *ui_;
  const SharedPtr<RemoteController> remote_;

  // Timer for checking network connection.
  QTimer *timer;

  // Set up incoming connection.
  QTcpServer *server;

  /* Check for active network connection. Set up port and
   * check if set for local only connections */
  bool active_network_connection();

  void startNetworkConnection();
  void setLocalOnly();

private Q_SLOTS:
  void on_enableRemote_clicked(bool checked);
  void on_enableRemote_toggled(bool checked);
  void checkNetworkConnection();

  // Network port objects.
  void onNewConnection();
  void onReadyRead();
  void onDisconnect();


};

#endif // REMOTECONTROLLERSETTINGSPAGE_H
