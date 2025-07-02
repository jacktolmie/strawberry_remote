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

#include <QObject>
#include <QString>
#include <QStringList>

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

private Q_SLOTS:

void on_enableRemote_clicked(bool checked);
void on_enableRemote_toggled(bool checked);
};

#endif // REMOTECONTROLLERSETTINGSPAGE_H
