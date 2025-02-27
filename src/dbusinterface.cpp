#include "dbusinterface.h"
#include "dbushelper.h"
#include <iostream>
#include <QJsonDocument>

DBusInterface::DBusInterface(Logger& l):logger(l)
{
    QDBusConnection::sessionBus().registerService("org.deepin.dde.digitalWellbeing");
    qDebug()<<"register"<<QDBusConnection::sessionBus().registerObject("/org/deepin/dde/digitalWellbeing","org.deepin.dde.digitalWellbeing",this,QDBusConnection::ExportAllSlots|QDBusConnection::ExportAllSignals);
}
QString DBusInterface::getDailyStatJson(){
    qWarning()<<"DBusInterface::getDailyStatJson";
    auto res=logger.readDailyRecordJson();
    return QJsonDocument(res).toJson(QJsonDocument::Compact);
}
QString DBusInterface::getWeeklyStatJson(){
    qWarning()<<"DBusInterface::getWeeklyStatJson";
    auto res=logger.readWeeklyRecordJson();
    return QJsonDocument(res).toJson(QJsonDocument::Compact);
}
QString DBusInterface::getPerAppStatJson(){
    qWarning()<<"DBusInterface::getPerAppStatJson";
    auto res=logger.readPerAppDailyStatJson();
    return QJsonDocument(res).toJson(QJsonDocument::Compact);
}
QString DBusInterface::getBasicStatJson(){
    qWarning()<<"DBusInterface::getBasicStatJson";
    auto res=logger.readBasicStatJson();
    return QJsonDocument(res).toJson(QJsonDocument::Compact);
}
