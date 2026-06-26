#include "remotesettings.h"

#include "constants/remotesettings.h"
#include "core/settings.h"

using namespace RemoteControllerSettings;

RemoteSettings::RemoteSettings([[maybe_unused]]QObject *parent)
{
  values = Values();
  RemoteSettings::getValues();
}

void RemoteSettings::getValues()
{
  Settings s;
  s.beginGroup(kSettingsGroup);

  values.activeNetwork =   s.value(kActiveNetwork, false).toBool();
  values.authRequired =    s.value(kUseAuthentication, false).toBool();
  values.hashedPassword =  s.value(kHashedPassword, QByteArray()).toByteArray();
  values.password =        s.value(kPassword, QString()).toString();
  values.portNumber =      s.value(kPort, 5000).toInt();
  values.remoteEnabled =   s.value(kRemoteEnabled, false).toBool();

  s.endGroup();
}

void RemoteSettings::saveValues()
{
  Settings s;
  s.beginGroup(kSettingsGroup);

  s.setValue(kActiveNetwork, values.activeNetwork);
  s.setValue(kHashedPassword, values.hashedPassword);
  s.setValue(kPassword, values.password);
  s.setValue(kPort, values.portNumber);
  s.setValue(kRemoteEnabled, values.remoteEnabled);
  s.setValue(kUseAuthentication, values.authRequired);

  s.endGroup();

  Q_EMIT RemoteSettings::sendValues(values);
}

void RemoteSettings::receiveValues(const Values& sentValues)
{
  this->values = sentValues;
  saveValues();
}

void RemoteSettings::updateNetworkStatus(bool isActive)
{
  if (values.activeNetwork != isActive) Q_EMIT RemoteSettings::networkStatusChanged(isActive);
}
