#include "appinfo.h"
#include "dbushelper.h"
#include <QLocale>

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
//    qDebug()<<"getAppInfoFromBAMF::"<<path;
    QString desktopFile=dcall(QDBusMessage::createMethodCall("org.ayatana.bamf",path,"org.ayatana.bamf.application","DesktopFile"),{QVariant("")}).takeFirst().toString();
    if(!desktopFile.length()){
        const auto &args=dcall(QDBusMessage::createMethodCall("org.ayatana.bamf",path,"org.ayatana.bamf.view","Children"),{QStringList()}).first().toStringList();
        qDebug()<<"view.children"<<args;
        for(QString subpath:args){
            uint32_t pid=dcall(QDBusMessage::createMethodCall("org.ayatana.bamf",subpath,"org.ayatana.bamf.window","GetPid"),{QVariant(0)}).first().toUInt();
            QFile environ("/proc/"+QString::number(pid)+"/environ");
            environ.open(QIODevice::ReadOnly);
            auto env=environ.readAll().split('\x00');
            qDebug()<<subpath<<" pid="<<pid;//<<" environ:"<<env;
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

// from https://github.com/linuxdeepin/dde-application-manager/blob/master/src/global.h
inline QString escapeToObjectPath(const QString &str)
{
    if (str.isEmpty()) {
        return "_";
    }

    auto ret = str;
    QRegularExpression re{R"([^a-zA-Z0-9])"};
    auto matcher = re.globalMatch(ret);
    while (matcher.hasNext()) {
        auto replaceList = matcher.next().capturedTexts();
        replaceList.removeDuplicates();
        for (const auto &c : replaceList) {
            auto hexStr = QString::number(static_cast<uint>(c.front().toLatin1()), 16);
            ret.replace(c, QString{R"(_%1)"}.arg(hexStr));
        }
    }
    return ret;
}

void AppInfo::getAppInfoFromLauncher(){
    auto readProp = [&](QString prop){
        QDBusInterface iface("org.desktopspec.ApplicationManager1","/org/desktopspec/ApplicationManager1/"+escapeToObjectPath(m_id),"org.freedesktop.DBus.Properties");
        auto resp= iface.call("Get","org.desktopspec.ApplicationManager1.Application",prop);
        auto dbusArg = resp.arguments().first().value<QDBusVariant>();
        // 解析嵌套的variant结构
        QVariant innerVariant = dbusArg.variant();
        // 解析最终的a{ss}结构（字符串字典）
        QMap<QString, QString> resultMap;
        if (innerVariant.userType() == qMetaTypeId<QDBusArgument>()) {
            QDBusArgument mapArg = innerVariant.value<QDBusArgument>();
            mapArg >> resultMap;
        } else if (innerVariant.canConvert(QMetaType::QVariantMap)) {
            QVariantMap vm = innerVariant.toMap();
            for (auto it = vm.begin(); it != vm.end(); ++it) {
                resultMap.insert(it.key(), it.value().toString());
            }
        }

        // 现在resultMap包含{"default": "Microsoft Edge (dev)"}
        qDebug() << "Extracted map:" << resultMap;
        return resultMap;
    };
    auto names = readProp("Name");
    m_name = names.value(QLocale::system().name(),
        names.value("default",m_id));
    m_icon = readProp("Icons").value("Desktop Entry","unknown");
}
