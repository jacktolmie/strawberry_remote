#include "remotecommands.h"


RemoteCommands::RemoteCommands()
{

}

QString RemoteCommands::processCommand(const QString& command)
{

}

void RemoteCommands::processLine(const QString& line)
{
  // Do whatever
  Q_EMIT RemoteCommands::forwardToPlayer(line, {}); // Figure out what to emit.
}
