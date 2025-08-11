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

  // Set up timer.
  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &RemoteControllerSettingsPage::checkNetworkConnection);

  // Set up port connection
  server = new QTcpServer(this);
  connect(server, &QTcpServer::newConnection, this, &RemoteControllerSettingsPage::onNewConnection);
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
    // Enable settings if enabled is checked. Open port etc.
    ui_->settingsGroupBox->isEnabled();
    server->listen(QHostAddress::Any, ui_->portSpinBox->value());
    qDebug()<< "on enableremote clicked called. Server listening? " << server->isListening();
  }
  else {
      qDebug() << "Server closed? " << server->isListening();
      server->close();
  }
}

void RemoteControllerSettingsPage::on_enableRemote_toggled(bool checked)
{
  // Timer to check for network connection. Checks every 5 seconds
  qDebug() << "on enabled remote toggled called " << checked;

  ui_->settingsGroupBox->setEnabled(checked);

  if(checked)timer->start(5000);

  if(!checked) timer->stop();

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

void RemoteControllerSettingsPage::onNewConnection()
{
  qDebug() << "onNewConnection called";
  QTcpSocket *socket = server->nextPendingConnection();
  connect(socket, &QTcpSocket::readyRead, this, &RemoteControllerSettingsPage::onReadyRead);
  connect(socket, &QTcpSocket::disconnected, this, &RemoteControllerSettingsPage::onDisconnect);
}

void RemoteControllerSettingsPage::onReadyRead(){
  QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
  QByteArray data = socket->readAll();
}

void RemoteControllerSettingsPage::onDisconnect(){
  QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
  socket->deleteLater();
}
void RemoteControllerSettingsPage::setLocalOnly()
{

}
