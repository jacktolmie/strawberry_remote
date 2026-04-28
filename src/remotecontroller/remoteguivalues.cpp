#include "remoteguivalues.h"
#include "core/logging.h"
#include "core/player.h"
#include "constants/timeconstants.h"

using namespace Qt::Literals::StringLiterals;

RemoteGuiValues::RemoteGuiValues(const Application* app, QObject *parent)// const Ui_MainWindow* ui, QObject *parent)
  : QObject{parent},
    app_{app}{}

void RemoteGuiValues::getUpdates(QTcpSocket *client){
  qDebug() << "RemoteGuiValues::getUpdates called";
  QJsonObject response;
  response[u"event"_s] = u"gui_updates"_s;
  response[u"volume"_s] = static_cast<qint32>(app_->player()->GetVolume());
  response[u"current_time"_s] = app_->player()->engine()->position_nanosec() / kNsecPerMsec;
  response[u"playing"_s] = (app_->player()->GetState() == EngineBase::State::Playing)? true : false;

  Q_EMIT RemoteGuiValues::sendCurrentStatus(client, response);
}

void RemoteGuiValues::triggerUpdate(QTcpSocket *client)
{
  RemoteGuiValues::getUpdates(client);
}
