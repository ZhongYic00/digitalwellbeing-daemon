#ifndef USAGESTATITEM_H
#define USAGESTATITEM_H

#include <QObject>
#include <QtCore>

//class UsageStatItem : public QObject
struct UsageStatItem
{
//    Q_OBJECT
//public:
//    explicit UsageStatItem(QObject *parent = nullptr);
//    Q_PROPERTY(QString appName READ appName WRITE setAppName)
//    Q_PROPERTY(QDateTime begin READ begin WRITE setBegin)
//    Q_PROPERTY(QDateTime end READ end WRITE setEnd)

    QString appName;
    QDateTime begin;
    QDateTime end;

    friend QDataStream& operator <<(QDataStream& stream,const UsageStatItem& item){
        stream<<item.appName<<item.begin<<item.end;
        return stream;
    }
    friend QDataStream& operator >>(QDataStream& stream, UsageStatItem& item){
        stream>>item.appName>>item.begin>>item.end;
        return stream;
    }
    friend QTextStream& operator <<(QTextStream& stream,const UsageStatItem& item){
        stream<<item.appName<<' '<<item.begin.toString()<<' '<<item.end.toString();
        return stream;
    }
    QJsonObject toJSON() const {
        QJsonObject obj;
        obj.insert("appName",QJsonValue(appName));
        obj.insert("begin",QJsonValue(begin.toString()));
        obj.insert("end",QJsonValue(end.toString()));
        return obj;
    }
//signals:

};

#endif // USAGESTATITEM_H
