#include "remoteguivalues.h"
#include "core/logging.h"
#include "core/player.h"
#include "constants/timeconstants.h"

RemoteGuiValues::RemoteGuiValues(const Application* app, QObject *parent)// const Ui_MainWindow* ui, QObject *parent)
  : QObject{parent},
    app_{app}{}

void RemoteGuiValues::getUpdates(QTcpSocket *client){
  qDebug() << "RemoteGuiValues::getUpdates called";
  QJsonObject response;
  response[QStringLiteral("command")] = QStringLiteral("gui_updates");
  response[QStringLiteral("volume")] = static_cast<qint32>(app_->player()->GetVolume());
  response[QStringLiteral("current_time")] = app_->player()->engine()->position_nanosec() / kNsecPerMsec;
  response[QStringLiteral("playing")] = (app_->player()->GetState() == EngineBase::State::Playing)? true : false;

  Q_EMIT RemoteGuiValues::sendCurrentStatus(client, response);
}

void RemoteGuiValues::triggerUpdate(QTcpSocket *client)
{
  RemoteGuiValues::getUpdates(client);
}
