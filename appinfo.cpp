#include "appinfo.h"
#include "dbushelper.h"

AppInfo::AppInfo(QString id):m_id(id){
    getAppInfoFromLauncher();
}
AppInfo::AppInfo(QString id,QString name):m_id(id),m_name(name),m_icon(id),m_desktopFile(id+".desktop"){

}
void AppInfo::setDesktopFile(QString desktopFile){
    m_desktopFile=desktopFile;
}
AppInfo AppInfo::getAppInfoFromBAMFPath(QString path){
    if(!path.length())return  AppInfo();
//    qWarning()<<"getAppInfoFromBAMF::"<<path;
    QString desktopFile=dcall(QDBusMessage::createMethodCall("org.ayatana.bamf",path,"org.ayatana.bamf.application","DesktopFile"),{QVariant("")}).takeFirst().toString();
    if(!desktopFile.length()){
        const auto &args=dcall(QDBusMessage::createMethodCall("org.ayatana.bamf",path,"org.ayatana.bamf.view","Children"),{QStringList()}).first().toStringList();
        qWarning()<<args;
        for(QString subpath:args){
            uint32_t pid=dcall(QDBusMessage::createMethodCall("org.ayatana.bamf",subpath,"org.ayatana.bamf.window","GetPid"),{QVariant(0)}).first().toUInt();
            QFile environ("/proc/"+QString::number(pid)+"/environ");
            environ.open(QIODevice::ReadOnly);
            auto env=environ.readAll().split('\x00');
            qWarning()<<subpath<<" pid="<<pid;//<<" environ:"<<env;
            for(auto kv:env){
                auto key=kv.left(kv.indexOf('='));
                if(key=="GIO_LAUNCHED_DESKTOP_FILE")
                    desktopFile=kv.mid(kv.indexOf('=')+1);
            }
        }
    }
    return AppInfo(
                desktopFile2Id(desktopFile)
//                dcall(QDBusMessage::createMethodCall("org.ayatana.bamf",path,"org.ayatana.bamf.view","Name"),{QVariant("-")}).takeFirst().toString()
                );
}
AppInfo AppInfo::getAppInfoFromId(QString id){
    return AppInfo(id);
}
void AppInfo::getAppInfoFromLauncher(){
    auto msg=QDBusMessage::createMethodCall("com.deepin.dde.daemon.Launcher","/com/deepin/dde/daemon/Launcher","com.deepin.dde.daemon.Launcher","GetItemInfo");
    msg<<m_id;
    auto resp=dcall(msg,{});
    if(!resp.length()){
        m_name=m_id;
        return;
    }
    const auto &args=resp.takeFirst().value<QDBusArgument>();
    args.beginStructure();
    QString id;
    args>>m_desktopFile>>m_name>>id>>m_icon>>m_category>>m_timeInstalled;
    args.endStructure();
}
