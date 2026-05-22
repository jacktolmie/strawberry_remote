#include "remoteguivalues.h"
#include "core/player.h"
#include "constants/timeconstants.h"
#include "remotecontroller/remotejsoncreator.h"
#include "remotecontroller/remoteplaylist.h"
#include "remotecontroller/remotetypes.h"

using namespace Qt::Literals::StringLiterals;
using namespace RemoteTypes;

RemoteGuiValues::RemoteGuiValues(const Application* app, QObject *parent)
  : QObject{parent},
    app_{app},
    playlist(RemotePlaylist(app_, this))
{}

// void RemoteGuiValues::getUpdates(const QTcpSocket *client) const{
QJsonObject RemoteGuiValues::getUpdates() const{
  qDebug() << "RemoteGuiValues::getUpdates called";

  return RemoteJsonCreator::createResponse({
    field(MessageType::EVENT, toString(MessageType::EVENT)),
    field(Event::EVENT, toString(Event::GUI_UPDATES)),
    field(Arguments::VOLUME, static_cast<qint32>(app_->player()->GetVolume())),
    field(Arguments::CURRENT_TIME, app_->player()->engine()->position_nanosec() / kNsecPerMsec),
    field(Arguments::PLAYING, (app_->player()->GetState() == EngineBase::State::Playing)? true : false),
    field(Arguments::PLAYLISTS, playlist.sendAllPlaylists())
  });
}

// void RemoteGuiValues::triggerUpdate(QTcpSocket *client) const
QJsonObject RemoteGuiValues::triggerUpdate() const {
  return getUpdates();
}
