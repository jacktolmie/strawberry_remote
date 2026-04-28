#include <QStringView>
#include "remotejsoncreator.h"

namespace RemoteJsonCreator{

  QJsonObject createResponse(const QString& domain, const QString& message){
    qInfo() << "CreateResponse called with two arguments";
    QJsonObject response;
    response[u"type"] = domain;
    response[domain] = message;
    return response;
    // return QJsonObject{{domain, message}};
  }

  QJsonObject createResponse(QMap<QString, QString>&& sentMap){
    qInfo() << "CreateResponse called with map";
    QJsonObject response;
    response[u"type"] = sentMap.firstKey();
    for(const auto& [key,value]: sentMap.asKeyValueRange()){
      response[key] = value;
    }
    return response;
  }
}
