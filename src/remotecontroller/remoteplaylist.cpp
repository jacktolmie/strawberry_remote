#include "remoteplaylist.h"


RemotePlaylist::RemotePlaylist(Application* app, QObject *parent)
    : QObject{parent}
{

}

void RemotePlaylist::processCommand(const QString& command, [[maybe_unused]] const QStringList& args)
{
  qDebug() << "RemotePlaylist::processCommand called";

}

QMap<QString, std::function<void(const QStringList&)>> RemotePlaylist::createCommandMap()
{
  // auto
}
