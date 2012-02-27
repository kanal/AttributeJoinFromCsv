#include "AttributeJoinFromCsv.h"
#include "qgisinterface.h"
#include <QAction>
#include <QTextCodec>
#include <QMessageBox>
#include "AttributeJoinDlg.h"

#ifdef WIN32
#define QGISEXTERN extern "C" __declspec( dllexport )
#else
#define QGISEXTERN extern "C"
#endif

AttributeJoinFromCsv::AttributeJoinFromCsv(QgisInterface* iface)
    : mIface(iface), mAction(0)
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
}

AttributeJoinFromCsv::~AttributeJoinFromCsv()
{

}

void AttributeJoinFromCsv::initGui()
{
    mAction = new QAction(QIcon(":icons/icons/mmqgis_attribute_join.png"),tr("&attribute join"), this);
    connect(mAction, SIGNAL(activated()), this, SLOT(joinFromCsv()));
    mIface->addToolBarIcon(mAction);
    mIface->addPluginToMenu(tr("&attribute join"), mAction);
}

void AttributeJoinFromCsv::unload()
{
    mIface->removeToolBarIcon(mAction);
    mIface->removePluginMenu(tr("&attribute join"), mAction);
    delete mAction;
}

void AttributeJoinFromCsv::joinFromCsv()
{
    AttributeJoinDlg dlg(mIface);
    dlg.exec();
}

QGISEXTERN QgisPlugin* classFactory(QgisInterface* iface)
{
    return new AttributeJoinFromCsv(iface);
}

QGISEXTERN QString name()
{
    return "attribute join from csv plugin";
}

QGISEXTERN QString description()
{
//    return "シェイプファイルにＣＳＶから属性を追加する";
    return "attribute join to shape from csv";
}

QGISEXTERN QString version()
{
    return "0.9";
}

// Return the type (either UI or MapLayer plugin)
QGISEXTERN int type()
{
    return QgisPlugin::UI;
}

// Delete ourself
QGISEXTERN void unload(QgisPlugin* theAttributeJoinFromCsv)
{
    delete theAttributeJoinFromCsv;
}

