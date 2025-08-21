#ifndef REMOTESETTINGS_H
#define REMOTESETTINGS_H

#include <QObject>
#include <QVector>
#include <QString>

struct Values
{
  // Variables for Remote Controller settings
  bool        activeNetwork;
  bool        authRequired;
  QByteArray  hashedPassword;
  QString     password;
  int         portNumber;
  bool        remoteEnabled;
};

class RemoteSettings: public QObject
{
  Q_OBJECT
public:
    RemoteSettings(QObject *parent = nullptr);
    ~RemoteSettings() = default;

  Values values;

private:
  void getValues();
  void saveValues();
  void getUpdates(Values& sentValues);
  void sendUpdates();

Q_SIGNALS:
  void sendValues(const Values& values);

public Q_SLOTS:
  void receiveValues(const Values& values);
};

#endif // REMOTESETTINGS_H
