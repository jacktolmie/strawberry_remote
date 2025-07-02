#include <QObject>
#include <QString>

#include "remotecontrollersettingspage.h"
#include "ui_remotecontrollersettingspage.h"
#include "constants/remotesettings.h"
#include "core/iconloader.h"
#include "core/settings.h"
#include "includes/shared_ptr.h"
#include "settings/settingspage.h"

using namespace Qt::Literals::StringLiterals;

RemoteControllerSettingsPage::RemoteControllerSettingsPage(SettingsDialog *dialog, QWidget *parent)
  : SettingsPage(dialog, parent),
    ui_(new Ui::RemoteControllerSettingsPage)
{
  ui_->setupUi(this);
  ui_->settingsGroupBox->setEnabled(ui_->enableRemote->isChecked());
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
  if(checked) ui_->settingsGroupBox->isEnabled();
}


void RemoteControllerSettingsPage::on_enableRemote_toggled(bool checked)
{
  ui_->settingsGroupBox->setEnabled(checked);
}

