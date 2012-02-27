#ifndef ATTRIBUTEJOINDLG_H
#define ATTRIBUTEJOINDLG_H

#include <QDialog>
#include <qgisplugin.h>

class QgsVectorLayer;

namespace Ui {
    class AttributeJoinDlg;
}

class AttributeJoinDlg : public QDialog
{
    Q_OBJECT

public:
    explicit AttributeJoinDlg(QgisInterface* iface,QWidget *parent = 0);
    ~AttributeJoinDlg();

private slots:
    void run();

private:
    Ui::AttributeJoinDlg *ui;
    QgisInterface* mIface;
    QgsVectorLayer* vectorLayer(QString layerName);
};

#endif // ATTRIBUTEJOINDLG_H
