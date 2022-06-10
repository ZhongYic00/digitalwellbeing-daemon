#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H

#include <QObject>
#include "logger.h"

class DBusInterface: public QObject
{
    Q_OBJECT
public:
    DBusInterface(Logger& l);
public slots:
    QString getDailyStatJson();
    QString getWeeklyStatJson();
    QString getPerAppStatJson();
    QString getBasicStatJson();
private:
    Logger& logger;
};

#endif // DBUSINTERFACE_H
