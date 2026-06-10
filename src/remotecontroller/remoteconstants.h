#ifndef REMOTECONSTANTS_H
#define REMOTECONSTANTS_H

#include <QStringList>

namespace remoteconstants
{
  // Lambda to check if args contains an int.
  inline constexpr auto parseUintArg = [](const QStringList& args, bool& ok)-> quint32 {
    if (args.isEmpty()){
      ok = false;
      return 0;
    }
    return args.first().toUInt(&ok);
  };

  // Lambda to check if args contains a string.
  inline constexpr auto parseStringArg = [](const QStringList& args, bool& ok)-> QString {
    if (args.isEmpty()){
      ok = false;
      return QString();
    }
    return QString(args.first());
  };

}
#endif // REMOTECONSTANTS_H
