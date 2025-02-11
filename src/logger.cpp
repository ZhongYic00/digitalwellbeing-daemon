#include "logger.h"
#include <QStandardPaths>
#include <iostream>
//#include <DtkCores>
#include "appinfo.h"
// #include <DDBusSender>

Logger::Logger():
    dataDirPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)),
    dailyLogPath(dataDirPath+"/daily-behaviour.db"),
    dailyLogPathArchive(dailyLogPath+".1"),
    longTermLogPath(dataDirPath+"/long-term-behaviour.log")
{
    QDir dir(dataDirPath);
    if(!dir.exists())
        dir.mkpath(dataDirPath);
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dailyLogPath);
    db.open();
    executor=QSqlQuery(db);
    std::cout << db.databaseName().toStdString() << std::endl;
    executor.exec("CREATE TABLE IF NOT EXISTS EventsRaw(appid text,tstart int,tend int)");
    executor.exec("CREATE VIEW IF NOT EXISTS EventsDaily AS select appid,max(0,tstart-strftime('%s',datetime('now','localtime','start of day','utc'))) as tsoff,tend-strftime('%s',datetime('now','localtime','start of day','utc')) as teoff from EventsRaw where teoff>=0");
    executor.exec("CREATE VIEW IF NOT EXISTS PerAppSumDaily AS select appid,sum(teoff-tsoff) AS time,count(appid) AS freq FROM EventsDaily GROUP BY appid ORDER BY time DESC");
    executor.exec("SELECT name FROM sqlite_master WHERE type='table'");
    while(executor.next()){
        qDebug()<<executor.value(0).toString();
    }
    std::cerr<<"Logger::Logger() dailyLogPath="<<dailyLogPath.toStdString()<<std::endl;
}
void Logger::appendRecord(const UsageStatItem &item){
    std::cerr<<"Logger::appendRecord()"<<std::endl;
    executor.prepare("insert into EventsRaw values(:appid,:tstart,:tend)");
    executor.bindValue(":appid",item.appName);
    executor.bindValue(":tstart",item.begin.toTime_t());
    executor.bindValue(":tend",item.end.toTime_t());
    std::cerr<<"appendRecord::"<<executor.exec();
}
QJsonArray Logger::readDailyRecordJson(){
    QJsonArray rt;
    executor.exec("select appid,tsoff,teoff from EventsDaily");
    while(executor.next()){
        QJsonObject obj;
        obj["id"]=executor.value(0).toString();
        obj["tsoff"]=executor.value(1).toInt();
        obj["teoff"]=executor.value(2).toInt();
        rt.append(obj);
    }
    return rt;
}
QJsonArray Logger::readWeeklyRecordJson(){
    QJsonArray rt;
    return rt;
}
QJsonArray Logger::readPerAppDailyStatJson(){
    QJsonArray rt;
    executor.exec("select appid,time,freq from PerAppSumDaily");
    while(executor.next()){
        QJsonObject obj;
        QString id;
        obj["id"]=id=executor.value(0).toString();
        auto appinfo=AppInfo::getAppInfoFromId(id);
        obj["name"]=appinfo.name();
        obj["icon"]=appinfo.icon();
        obj["time"]=executor.value(1).toInt();
        obj["freq"]=executor.value(2).toInt();
        if(id!="dde-lock")
            rt.append(obj);
    }
    return rt;
}
QJsonObject Logger::readBasicStatJson(){
    QJsonObject rt;
    executor.exec("select sum(time) as totalTime from PerAppSumDaily where appid!='dde-lock'");
    executor.next();
    rt["totalTime"]=executor.value(0).toInt();
    executor.exec("select appid from PerAppSumDaily order by time desc limit 2");
    while(executor.next())if(executor.value(0).toString()!="dde-lock")break;
    rt["longestUsedApp"]=AppInfo::getAppInfoFromId(executor.value(0).toString()).name();
    return rt;
}
