/*
* Strawberry Music Player
* Copyright 2024, Jonas Kvinge <jonas@jkvinge.net>
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

#ifndef REMOTECONTROLLERSETTINGS_H
#define REMOTECONTROLLERSETTINGS_H

namespace RemoteControllerSettings {

constexpr char kSettingsGroup[]     = "RemoteController";
constexpr char kRemoteEnabled[]     = "remote_enabled";
constexpr char kPassword[]          = "password";
constexpr char kHashedPassword[]    = "hashed_password";
constexpr char kPort[]              = "port";
constexpr char kUseAuthentication[] = "use_authentication";
constexpr char kActiveNetwork[]     = "active_network";
}  // namespace

#endif  // REMOTECONTROLLERSETTINGS_H
