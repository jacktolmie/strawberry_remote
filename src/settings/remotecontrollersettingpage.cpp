#include "remotecontrollersettingpage.h"
#include "ui_remotecontrollersettingpage.h"

RemoteControllerSettingPage::RemoteControllerSettingPage(SettingsDialog *dialog, QWidget *parent)
    : SettingsPage(dialog, parent),
      ui(new Ui::RemoteControllerSettingPage)
{
    ui->setupUi(this);
}

RemoteControllerSettingPage::~RemoteControllerSettingPage()
{
    delete ui;
}


void RemoteControllerSettingPage::Load(){

}

void RemoteControllerSettingPage::Save(){

}
