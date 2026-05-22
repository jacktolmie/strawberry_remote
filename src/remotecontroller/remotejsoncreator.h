#ifndef REMOTEJSONCREATOR_H
#define REMOTEJSONCREATOR_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QJsonObject>

namespace RemoteJsonCreator{
  // QJsonObject createResponse(const QString& domain, const QString& message);
  // QJsonObject createResponse(std::initializer_list<std::pair<QString, QJsonValue>> fields = {});
  QJsonObject createResponse(std::initializer_list<std::pair<QString, QJsonValue>> fields);

}

#endif // REMOTEJSONCREATOR_H
