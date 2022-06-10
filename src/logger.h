#ifndef LOGGER_H
#define LOGGER_H
#include <QFile>
#include <QDir>
#include <QList>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql>
#include "usagestatitem.h"

class Logger
{
public:
    Logger();
    void appendRecord(const UsageStatItem& item);
    QJsonArray readDailyRecordJson();
    QJsonArray readWeeklyRecordJson();
    QJsonArray readPerAppDailyStatJson();
    QJsonObject readBasicStatJson();

    const QString dataDirPath;
    const QString dailyLogPath;
    const QString dailyLogPathArchive;
    const QString longTermLogPath;
private:
    QSqlDatabase db;
    QSqlQuery executor;
};

#endif // LOGGER_H
