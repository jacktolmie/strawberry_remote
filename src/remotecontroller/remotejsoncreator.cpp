#include <QStringView>
#include "remotejsoncreator.h"

using namespace Qt::Literals::StringLiterals;

namespace RemoteJsonCreator{
  QJsonObject createResponse(std::initializer_list<std::pair<QString, QJsonValue>> fields) {
      if (fields.size() == 0) return QJsonObject({{u"error"_s, u"empty_fields"_s}});
      QJsonObject response;
      for (const auto& [key, value] : fields) {
          response.insert(key, value);
      }
      return response;
  }
}
