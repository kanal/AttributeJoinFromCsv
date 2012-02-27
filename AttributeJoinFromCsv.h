#ifndef ATTRIBUTEJOINFROMCSV_H
#define ATTRIBUTEJOINFROMCSV_H

#include <qgisplugin.h>
#include <QObject>
class QAction;

class AttributeJoinFromCsv: public QObject, public QgisPlugin {

    Q_OBJECT

public:
    AttributeJoinFromCsv(QgisInterface* iface);
    ~AttributeJoinFromCsv();
    void initGui();
    void unload();

private:
    QgisInterface* mIface;
    QAction* mAction;

private slots:
    void joinFromCsv();

};

#endif // ATTRIBUTEJOINFROMCSV_H
