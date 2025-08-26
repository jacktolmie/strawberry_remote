#include "remotecontrollersettingspage.h"
#include "ui_remotecontrollersettingspage.h"
#include "core/iconloader.h"
// #include "core/settings.h"
#include "settings/settingspage.h"

using namespace Qt::Literals::StringLiterals;
// using namespace RemoteControllerSettings;

RemoteControllerSettingsPage::RemoteControllerSettingsPage(SettingsDialog *dialog, SharedPtr<RemoteSettings> sentData, QWidget *parent)
  : SettingsPage(dialog, parent),
    ui_(new Ui::RemoteControllerSettingsPage),
    data_(sentData)
{
  ui_->setupUi(this);
  Init(this);
  setWindowIcon(IconLoader::Load(u"remote_control"_s, true, 0, 32));

  // Connect the password setting section if password required checkbox is toggled.
  connect(ui_->authCodeCkBx, &QCheckBox::toggled, this, &RemoteControllerSettingsPage::setPassword);

  // Connect the enable group box if the checkbox is toggled.
  connect(ui_->enableRemote, &QCheckBox::toggled, this, &RemoteControllerSettingsPage::on_enableRemote_clicked);
}

RemoteControllerSettingsPage::~RemoteControllerSettingsPage()
{
  delete ui_;
}

void RemoteControllerSettingsPage::Load(){

  ui_->authCodeCkBx->setChecked(data_->values.authRequired);
  ui_->authCodeEdit->setText(data_->values.password);
  ui_->enableRemote->setChecked(data_->values.remoteEnabled);
  ui_->networkNotActive->setVisible(!data_->values.activeNetwork);
  ui_->portSpinBox->setValue(data_->values.portNumber);
  ui_->authCodeEdit->setEnabled(data_->values.authRequired);
  ui_->settingsGroupBox->setEnabled(data_->values.remoteEnabled);

  // Run check to enable/disable group box, and check network connection.
  RemoteControllerSettingsPage::on_enableRemote_clicked();

}

void RemoteControllerSettingsPage::Save(){

  // If any settings were changed, return values to remotesettings.
  if(
      data_->values.authRequired != ui_->authCodeCkBx->isChecked() ||
      data_->values.password != ui_->authCodeEdit->text() ||
      data_->values.portNumber != ui_->portSpinBox->value() ||
      data_->values.remoteEnabled != ui_->enableRemote->isChecked()
  ){

    qLog(Debug) << "Remote Controller settings changed. Emitting signal";
    data_->values.authRequired = ui_->authCodeCkBx->isChecked();
    data_->values.password = ui_->authCodeEdit->text();
    data_->values.portNumber = ui_->portSpinBox->value();
    data_->values.remoteEnabled = ui_->enableRemote->isChecked();
    data_->values.hashedPassword = QCryptographicHash::hash(ui_->authCodeEdit->text().toUtf8(), QCryptographicHash::Sha256);

    qLog(Debug) << "Remote Controller settings changed. Updating values.";
    data_->receiveValues(data_->values);
  }
}

void RemoteControllerSettingsPage::on_enableRemote_clicked()
{
  ui_->settingsGroupBox->setEnabled(ui_->enableRemote->isChecked());

  // If there is no active network, show warning.
  if(!data_->values.activeNetwork){
    QString fontColour = QStringLiteral("color: red;");
    ui_->networkNotActive->setStyleSheet(fontColour);
  }
  ui_->networkNotActive->setVisible(!data_->values.activeNetwork);
}

Ui::RemoteControllerSettingsPage* RemoteControllerSettingsPage::getUi(){
  return ui_;
}

void RemoteControllerSettingsPage::setPassword()
{
  ui_->authCodeEdit->setEnabled(ui_->authCodeCkBx->isChecked());
}
