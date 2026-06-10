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
  void networkStatusChanged(bool isActive);

public Q_SLOTS:
  void receiveValues(const Values& sentValues);
  void updateNetworkStatus(bool isActive);
};

#endif // REMOTESETTINGS_H
