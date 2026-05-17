#include "remoteguivalues.h"
#include "core/player.h"
#include "constants/timeconstants.h"
#include "remotecontroller/remotejsoncreator.h"

using namespace Qt::Literals::StringLiterals;

RemoteGuiValues::RemoteGuiValues(const Application* app, QObject *parent)// const Ui_MainWindow* ui, QObject *parent)
  : QObject{parent},
    app_{app}{}

void RemoteGuiValues::getUpdates(QTcpSocket *client){
  qDebug() << "RemoteGuiValues::getUpdates called";
  Q_EMIT RemoteGuiValues::sendCurrentStatus(client, RemoteJsonCreator::createResponse({
    {u"event"_s, u"gui_updates"_s},
    {u"volume"_s, static_cast<qint32>(app_->player()->GetVolume())},
    {u"current_time"_s, app_->player()->engine()->position_nanosec() / kNsecPerMsec},
    {u"playing"_s, (app_->player()->GetState() == EngineBase::State::Playing)? true : false}
  }));
}

void RemoteGuiValues::triggerUpdate(QTcpSocket *client)
{
  RemoteGuiValues::getUpdates(client);
}
