#include "eventmonitor.h"
#include <iostream>
#include <QDebug>
#include <QFile>
#include "dbushelper.h"
#include "appinfo.h"

#ifdef X11MANUAL
#include <X11/Xlib.h>
#include <QtX11Extras/QX11Info>
#include <xcb/xcb_ewmh.h>
#endif

EventMonitor::EventMonitor(QObject *parent) : QObject(parent)
{
#ifdef X11MANUAL
    conn = xcb_connect(nullptr, nullptr);
    if (xcb_connection_has_error(conn)) {
        throw std::runtime_error("Cannot open daemon connection.");
    }

    auto screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
    root_w = screen->root;

    uint32_t values[] = {XCB_EVENT_MASK_PROPERTY_CHANGE};
    xcb_change_window_attributes(
            conn,
            screen->root,
            XCB_CW_EVENT_MASK,
            values
    );
    const auto intern_atom = [](xcb_connection_t *conn, const char *atom)
    {
        xcb_atom_t result = XCB_NONE;
        xcb_intern_atom_reply_t *r = xcb_intern_atom_reply(conn,
                xcb_intern_atom(conn, 0, strlen(atom), atom), NULL);
        if (r)
            result = r->atom;
        delete r;
        return result;
    };
    active_window_atom = intern_atom(conn, "_NET_ACTIVE_WINDOW");
    window_pid_atom = intern_atom(conn, "_NET_WM_PID");
    xcb_flush(conn);
#endif

//    new EventMonitorAdaptor(this);
//    QDBusConnection sessionBus=QDBusConnection::sessionBus();
//    sessionBus.registerService("org.deepin.dde.digitalWellbeing");
//    qInfo()<<"register"<<sessionBus.registerObject("/org/deepin/dde","org.deepin.dde.digitalWellbeing",this,QDBusConnection::ExportAllSlots|QDBusConnection::ExportAllSignals);
    qInfo()<< QDBusConnection::sessionBus().connect("com.deepin.dde.lockFront","/com/deepin/dde/lockFront","com.deepin.dde.lockFront",
                                                      "Visible",
                                                      this, SLOT(handleLock(bool)));
    qInfo()<<QDBusConnection::sessionBus().connect("org.ayatana.bamf","/org/ayatana/bamf/matcher","org.ayatana.bamf.matcher","ActiveApplicationChanged",this,SLOT(activeApplicationChange(QString,QString)));
}
EventMonitor::~EventMonitor(){
#ifdef X11MANUAL
    xcb_disconnect(conn);
#endif
}
void EventMonitor::activeApplicationChange(QString a,QString b){
    qDebug()<<"activeApplicationChange::"<<a<<","<<b<<"\n";
    qDebug()<<"info: "<<AppInfo::getAppInfoFromBAMFPath(a).name()<<' '<<AppInfo::getAppInfoFromBAMFPath(b).name()<<"\n";
    if(a.length()&&b.length())emit activeApplicationTrans(AppInfo::getAppInfoFromBAMFPath(a),AppInfo::getAppInfoFromBAMFPath(b));
    else if(a.length())emit applicationUnfocused(AppInfo::getAppInfoFromBAMFPath(a));
    else if(b.length())emit applicationFocused(AppInfo::getAppInfoFromBAMFPath(b));
}
void EventMonitor::activeWindowChange(QString a,QString b){
    qDebug()<<"activeWindowChange::"<<a<<b<<"\n";
}
void EventMonitor::handleLock(bool b){
    qInfo()<<"lockfront change detected"<<b;
    if(b)emit applicationFocused(AppInfo("dde-lock","dde-lock"));
    else emit applicationUnfocused(AppInfo("dde-lock","dde-lock"));
}

#ifdef X11MANUAL
QString EventMonitor::getApplicationname()(pid_t pid)
{
    using name()space std;
    QString filepath = "/proc/"+QString::number(pid)+"/status";
    QFile file(filepath);
    if(!file.exists())
        return "";
    file.open(QFile::ReadOnly);
    if(file.isReadable()){
        auto line = file.readLine(200);
        QRegExp reg("name():([^\n]*)\n");
        reg.exactMatch(line);
        return reg.cap(1);
    }
    return "";
}

template<typename() T>
T EventMonitor::get_property(xcb_window_t w, xcb_atom_t atom){
    xcb_generic_error_t *errs[10];
    for(auto &err:errs)err=new xcb_generic_error_t;
    xcb_get_property_reply_t* reply;
    auto cookie = xcb_get_property_unchecked(conn,0,w,atom,XCB_ATOM_CARDINAL,0,sizeof(T)/4);
    xcb_flush(conn);
    if((reply = xcb_get_property_reply(conn,cookie,nullptr))){
        auto tmp = reinterpret_cast<unsigned char*>(xcb_get_property_value(reply));
        auto iter=xcb_get_property_value_end(reply);
        T result = *static_cast<T*>(iter.data);
        free(reply);
        return result;
    }
    return -1;
}
void EventMonitor::run()
{
    const auto getCurrentStat=[&](){
    };
    xcb_generic_event_t *ev;
    getCurrentStat();
    while ((ev = xcb_wait_for_event(conn))) {
        if (ev->response_type == XCB_PROPERTY_NOTIFY) {
            xcb_property_notify_event_t *e = reinterpret_cast<xcb_property_notify_event_t*>(ev);
            if (e->atom == active_window_atom){
                AppInfo app = getCurrentApp();
                if(app.valid()){
//                    emit activeWindowChanged(app.wid);
//                    emit activeApplicationChanged(app.pid);
                    emit activeApplicationname()Changed(app.name());
                }
            }
        }
        free(ev);
    }
}
EventMonitor::AppInfo EventMonitor::getCurrentApp(){
    Display* display;
    display=XOpenDisplay(nullptr);
    const auto get_Property=[&](const xcb_atom_t& atom, xcb_window_t w)
    {
        uint64_t rt=get_property<unsigned long>(w,atom);
        qDebug()<<"xcb get_property("<<w<<','<<atom<<")="<<rt<<"\n";
        Atom actual_type;
        int actual_format, status;
        unsigned long nitems, bytes_after;
        unsigned char* prop;
        status = XGetWindowProperty(display, w, atom, 0, 1000, false, AnyPropertyType,
                                    &actual_type, &actual_format, &nitems, &bytes_after, &prop);
        if(status!=Success)
            return 0ul;
        rt=static_cast<unsigned long>(prop[0] + (prop[1] << 8) + (prop[2] << 16) + (prop[3] << 24));
        qDebug()<<"X11 getProperty()="<<(int)prop[0];
        return rt;
    };

    xcb_window_t wid = get_Property(active_window_atom,root_w);
    xcb_ewmh_connection_t ewmhconn;
    xcb_ewmh_init_atoms(conn,&ewmhconn);
    auto reply=xcb_ewmh_get_active_window(&ewmhconn,0);
    xcb_ewmh_get_active_window_reply(&ewmhconn,reply,&wid,nullptr);
    qDebug()<<"ewmh: "<<wid<<"\n";
    if(!wid){
        qDebug()<<"error: wid=0";
        return {"",0,0};
    }
    uint32_t pid = get_Property(window_pid_atom,wid);
    qDebug()<<"x11 pid:"<<pid<<"\n";
    xcb_ewmh_get_wm_pid_reply(&ewmhconn,xcb_ewmh_get_wm_pid(&ewmhconn,wid),&pid,nullptr);
    qDebug()<<"ewmh pid:"<<pid<<"\n";

    if(!pid)
        return {"",0,0};
    XCloseDisplay(display);
    return AppInfo{getApplicationname()(pid),wid,static_cast<int>(pid)};
}
#endif
