#ifndef REMOTEJSONCREATOR_H
#define REMOTEJSONCREATOR_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QJsonObject>

namespace RemoteJsonCreator{
  QJsonObject createResponse(const QString& domain, const QString& message);
  QJsonObject createResponse(QMap<QString, QString>&& map);
}

#endif // REMOTEJSONCREATOR_H
