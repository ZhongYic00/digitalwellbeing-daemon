#ifndef EVENTMONITOR_H
#define EVENTMONITOR_H

#include <QObject>
#include <xcb/xcb.h>
#include "appinfo.h"
//#include <QDBusAbstractAdaptor>
//#include <QDBusConnection>
//#include <X11/Xlib.h>

#ifdef X11MANUAL
class EventMonitor : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.digitalWellbeing")
public:
    static inline const char *staticInterfaceName()
    { return "org.deepin.dde.digitalWellbeing"; }

    struct AppInfo{
        QString name;
        xcb_window_t wid;
        pid_t pid;
        inline bool valid(){return wid&&pid&&name!="";}

    };

    explicit EventMonitor(QObject *parent = nullptr);
    ~EventMonitor();
    void run();
    AppInfo getCurrentApp();
private:
    AppInfo getApplicationInfo(QString path);
    QString getApplicationName(pid_t pid);
    template<typename T>
    T get_property(xcb_window_t w, xcb_atom_t atom);

    xcb_connection_t* conn;
    xcb_window_t root_w;
    xcb_atom_t active_window_atom;
    xcb_atom_t window_pid_atom;
signals:
    void activeWindowChanged(xcb_window_t);
    void activeApplicationChanged(pid_t);
    void activeApplicationNameChanged(QString);
public slots:
    inline void testSlot(const QString &s)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(s);
//        return asyncCallWithArgumentList(QStringLiteral("testSlot"), argumentList);
    }
    void activeWindowChange(QString,QString);
    void activeApplicationChange(QString,QString);
private slots:
    void handleLock(bool b);
};
#endif
class EventMonitor : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.digitalWellbeing")
public:
    explicit EventMonitor(QObject *parent = nullptr);
    ~EventMonitor();
private:
    AppInfo getApplicationInfo(QString path);

signals:
    void applicationUnfocused(AppInfo);
    void applicationFocused(AppInfo);
    void activeApplicationTrans(AppInfo,AppInfo);
public slots:
    inline void testSlot(const QString &s)
    {
//        QList<QVariant> argumentList;
//        argumentList << QVariant::fromValue(s);
    }
    void activeWindowChange(QString,QString);
    void activeApplicationChange(QString,QString);
private slots:
    void handleLock(bool b);
};

#endif // EVENTMONITOR_H
