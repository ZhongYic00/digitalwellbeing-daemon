#include <QCoreApplication>
#include <QThread>
#include <QDateTime>
#include <iostream>
#include <QDBusInterface>
#include "eventmonitor.h"
#include "logger.h"
#include "dbusinterface.h"
#include <QJsonDocument>

int main(int argc, char *argv[])
{
//#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
//    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//#endif

    QCoreApplication app(argc, argv);
    Logger logger;
    EventMonitor monitor;
    DBusInterface intf(logger);

#ifdef EXTRACT_DATA
    auto res=logger.readDailyRecord();
    QJsonArray Jarray;
    for(auto i:res){
        Jarray.append(i.toJSON());
    }
    qDebug()<<QJsonDocument(Jarray).toJson();
    auto records=logger.readDailyRecord();
    QMap<QString,qint64> summary;
    for(auto item:records){
        if(!summary.contains(item.appName))summary[item.appName]=0;
        summary[item.appName]+=item.begin.secsTo(item.end);
    }
    QJsonArray jarray;
    for(auto item:summary.toStdMap()){
        QJsonObject obj;
        obj.insert("name",item.first);
        obj.insert("time",item.second);
        qDebug()<<item.first;
        jarray.append(obj);
    }
    qDebug()<<QJsonDocument(jarray).toJson();
    return 0;
#endif

    auto lst = QDateTime::currentDateTimeUtc();
#ifdef X11MANUAL
    QThread worker(&app);
    QString lstApp = monitor.getCurrentApp().name;
    monitor.moveToThread(&worker);
    QObject::connect(&worker,&QThread::finished,&monitor,&QObject::deleteLater);
    QObject::connect(&worker,&QThread::started,&monitor,&EventMonitor::run);

    worker.start();
#endif

    QDBusConnection dbus=QDBusConnection::sessionBus();
    AppInfo lstApp;
    const auto writelog=[&logger](QDateTime lst,QDateTime cur,AppInfo app){
        qDebug()<<"last running:"<<app.id()<<" "<<app.name()<<" from "<<lst.toString()<<" to "<<cur.toString();
        logger.appendRecord({app.id(),lst,cur});
    };
    QObject::connect(&monitor,&EventMonitor::activeApplicationTrans,[&writelog,&lst,&lstApp](AppInfo lastApp,AppInfo currentApp){
        qDebug()<<"active application trans:"<<lastApp.id()<<','<<currentApp.id();
        auto cur = QDateTime::currentDateTimeUtc();
        if(cur.toSecsSinceEpoch()-lst.toSecsSinceEpoch()>2)writelog(lst,cur,lastApp);
        lst = cur;
        lstApp = currentApp;
    });
    QObject::connect(&monitor,&EventMonitor::applicationUnfocused,[&writelog,&lst,&lstApp](AppInfo lastApp){
        qDebug()<<"application unfocused:"<<lastApp.id();
        auto cur = QDateTime::currentDateTimeUtc();
        if(cur.toSecsSinceEpoch()-lst.toSecsSinceEpoch()>2)writelog(lst,cur,lastApp);
        lst = cur;
        lstApp = AppInfo();
    });
    QObject::connect(&monitor,&EventMonitor::applicationFocused,[&writelog,&lst,&lstApp](AppInfo curApp){
        qDebug()<<"application focused:"<<curApp.id();
        auto cur = QDateTime::currentDateTimeUtc();
        if(cur.toSecsSinceEpoch()-lst.toSecsSinceEpoch()>2)writelog(lst,cur,lstApp);
        lst = cur;
        lstApp = curApp;
    });
//    qDebug()<<"dbus bamf"<<dbus.connect("org.ayatana.bamf","/org/ayatana/bamf/matcher","org.ayatana.bamf.matcher","ActiveWindowChanged",&monitor,SLOT(activeWindowChange(QString,QString)));
#ifndef RELEASE
    qDebug()<<"running";
#endif
    return app.exec();
}
