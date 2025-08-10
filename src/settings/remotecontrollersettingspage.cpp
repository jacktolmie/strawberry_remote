// #include <QObject>
// #include <QString>

#include "remotecontrollersettingspage.h"
#include "ui_remotecontrollersettingspage.h"
#include "constants/remotesettings.h"
#include "core/iconloader.h"
#include "core/settings.h"
#include "settings/settingspage.h"

using namespace Qt::Literals::StringLiterals;

RemoteControllerSettingsPage::RemoteControllerSettingsPage(SettingsDialog *dialog, QWidget *parent)
  : SettingsPage(dialog, parent),
    ui_(new Ui::RemoteControllerSettingsPage)
{
  ui_->setupUi(this);
  ui_->settingsGroupBox->setEnabled(ui_->enableRemote->isChecked());
  RemoteControllerSettingsPage::checkNetworkConnection();
  setWindowIcon(IconLoader::Load(u"remote_control"_s, true, 0, 32));
}

RemoteControllerSettingsPage::~RemoteControllerSettingsPage()
{
  delete ui_;
}

void RemoteControllerSettingsPage::Load(){
  Settings s;
  s.beginGroup(RemoteControllerSettings::kSettingsGroup);
}

void RemoteControllerSettingsPage::Save(){

}

void RemoteControllerSettingsPage::on_enableRemote_clicked(bool checked)
{
  if(checked) {
    ui_->settingsGroupBox->isEnabled();
    RemoteControllerSettingsPage::checkNetworkConnection();
  }
}

void RemoteControllerSettingsPage::on_enableRemote_toggled(bool checked)
{
  ui_->settingsGroupBox->setEnabled(checked);
}

bool RemoteControllerSettingsPage::active_network_connection()
{
  const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

  for(const auto& interface : interfaces){
    if(interface.flags().testFlags(QNetworkInterface::IsUp) && !interface.flags().testFlags(QNetworkInterface::IsLoopBack)){
      return true;
    }
  }
  return false;
}

void RemoteControllerSettingsPage::checkNetworkConnection()
{
  // For some reason I cannot create a QString without the fromLatin1 added. I get a QString private constructor error.
  QString fontColour = QString::fromLatin1("color: red;");
  ui_->networkNotActive->setStyleSheet(fontColour);
  ui_->networkNotActive->setVisible(!RemoteControllerSettingsPage::active_network_connection());
}
