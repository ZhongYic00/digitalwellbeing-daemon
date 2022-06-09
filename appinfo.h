#ifndef APPINFO_H
#define APPINFO_H

//#include <QObject>
#include <QtCore>

class AppInfo
{
//    Q_OBJECT
public:
    AppInfo(QString id="unknown");
    AppInfo(QString id,QString name);
//    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
//    Q_PROPERTY(QString desktopFile READ desktopFile WRITE setDesktopFile NOTIFY desktopFileChanged)
//    Q_PROPERTY(QString id READ id NOTIFY idChanged STORED false);

    inline QString name() const{return m_name;}
    inline QString desktopFile() const{return m_desktopFile;}
    void setDesktopFile(QString desktopFile);
    inline QString id() const{return m_id;}
    inline QString icon() const{return m_icon;}

    static AppInfo getAppInfoFromBAMFPath(QString path);
    static AppInfo getAppInfoFromId(QString id);
    static QString desktopFile2Id(QString desktopFile){
        return desktopFile.mid(desktopFile.lastIndexOf('/')+1).remove(".desktop");
    }

private:
    QString m_desktopFile;
    QString m_name;
    QString m_id;
    QString m_icon;
    qlonglong m_category;
    qlonglong m_timeInstalled;

    void getAppInfoFromLauncher();
//signals:
//    void nameChanged(QString);
//    void desktopFileChanged(QString);
//    void idChanged(QString);
};

#endif // APPINFO_H
