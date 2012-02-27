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

}

AttributeJoinFromCsv::~AttributeJoinFromCsv()
{

}

void AttributeJoinFromCsv::initGui()
{
    mAction = new QAction(tr("&attribute join"), this);
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
    return "シェイプファイルにＣＳＶから属性を追加する";
}

QGISEXTERN QString version()
{
    return "0.00001";
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

