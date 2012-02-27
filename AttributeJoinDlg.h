#ifndef ATTRIBUTEJOINDLG_H
#define ATTRIBUTEJOINDLG_H

#include <QDialog>
#include <qgisplugin.h>
#include <QgsField.h>

class QgsVectorLayer;
class ErrorLog;

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
    void joinProc();
    void selectCsvFile();
    void selectShapeFile();
    void selectUnjoinedFile();
    void changeLayer(int index);
    void changeDataType(int index);
    void changeCsvType();

private:
    void initUi();
    bool isExistField(QgsFieldMap fieldMap, const QString& fieldName);
    QgsField getField(QgsFieldMap fieldMap, const QString& fieldName);
    QString createSubsetString(const QgsField& field, QString text);
    QVariant createFieldValue(const QgsField& field, QString text);
    QString createFieldValueString(const QgsField& field, const QVariant& var);
    bool preJoinProc();
    bool loadCsvData(const QString& filename);
    void loadCsvHeader(const QString& filename);

private:
    Ui::AttributeJoinDlg *ui;
    QgisInterface* mIface;
    QgsVectorLayer* getVectorLayerByName(QString layerName);

    QString m_csvFileName;
    QString m_shapeFileName;
    QString m_unjoinedFileName;

    QString m_csvKeyName;
    QString m_layerKeyName;
    QgsField m_keyField;

    QgsVectorLayer *m_vecLayer;

    QgsFieldMap m_orgFieldMap;
    QgsFieldMap m_newFieldMap;

    QMap < QString, QStringList > m_csvData;
    QStringList m_csvKey;
    ErrorLog *m_errLog;
};

#endif // ATTRIBUTEJOINDLG_H
