#ifndef DBUSHELPER_H
#define DBUSHELPER_H

#include <QtDBus/QtDBus>

const auto dcall=[](QDBusMessage msg,QList<QVariant> defaultv=QList<QVariant>()){
    auto resp=QDBusConnection::sessionBus().call(msg);
//        qWarning()<<"dcall::("<<msg<<")"<<resp;
    return resp.type()==QDBusMessage::ReplyMessage?resp.arguments():defaultv;
};


#endif // DBUSHELPER_H
