#include <QStringView>
#include "remotejsoncreator.h"

using namespace Qt::Literals::StringLiterals;

namespace RemoteJsonCreator{

  QJsonObject createResponse(const QString& domain, const QString& message){
    QJsonObject response;
    response[u"type"] = domain;
    response[domain] = message;
    return response;
  }

  QJsonObject createResponse(std::initializer_list<std::pair<QString, QJsonValue>> fields){
    if(fields.size() == 0) return QJsonObject({{u"error"_s, u"empty_fields"_s}});

    QJsonObject response;
    response[u"type"] = fields.begin()->first;
    for (const auto& [key, value] : fields) {
            response.insert(key, value);
    }
    return response;
    // QJsonObject response;
    // response[u"type"] = sentMap.firstKey();
    // for(const auto& [key,value]: sentMap.asKeyValueRange()){
    //   response[key] = value;
    // }
    // return response;
  }
}
