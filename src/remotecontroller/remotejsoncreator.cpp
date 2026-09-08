#include <QStringView>
#include "remotejsoncreator.h"
#include "remotetypes.h"

using namespace Qt::Literals::StringLiterals;
using namespace RemoteTypes;

namespace RemoteJsonCreator{
  QJsonObject createResponse(std::initializer_list<std::pair<QString, QJsonValue>> fields) {
      if (fields.size() == 0) return QJsonObject({{toString(Arguments::ERROR), toString(Arguments::EMPTY_FIELDS)}});
      QJsonObject response;
      for (const auto& [key, value] : fields) {
          response.insert(key, value);
      }
      return response;
  }
}
