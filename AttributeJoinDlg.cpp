//QT include
#include <QFile>
#include <QDebug>

//QGIS include
#include <qgsvectorfilewriter.h>
#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectordataprovider.h>
#include <qgsfield.h>
#include <qgisinterface.h>

//include
#include "AttributeJoinDlg.h"
#include "ui_AttributeJoinDlg.h"
#include "csv.h"

AttributeJoinDlg::AttributeJoinDlg(QgisInterface* iface,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AttributeJoinDlg)
{
    ui->setupUi(this);
    mIface = iface;

    qDebug() << "aaa";
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(run()));

    QFile file( "D:\\001-1.csv" );
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            //qDebug()<< "cannot read file!";
    }

    QString line = file.readLine();
    QStringList header = CSV::parseLine(line);
    for(int i=0;i<header.count();i++)
        ui->csvfilefield->addItem(header.at(i));

    //	set title
    ui->tblCsvField->setRowCount(10);
    ui->tblCsvField->setColumnCount(2);
    QStringList headerLabels;
    headerLabels << "フィールド名" << "型";
    ui->tblCsvField->setHorizontalHeaderLabels(headerLabels);


    QTableWidgetItem *item[2];
    //	set sample data values.
    for (int nrow = 0; nrow<header.count(); nrow++ ) {
        item[0] = new QTableWidgetItem(header.at(nrow));
        item[1] = new QTableWidgetItem("文字列");

        for(int ncol=0;ncol<2;ncol++) {
            ui->tblCsvField->setItem(nrow, ncol, item[ncol]);
        }
    }

    //	fit of column size at contents.
    ui->tblCsvField->resizeColumnsToContents();


    file.close();

    mIface->addVectorLayer("D:\\test.shp", "test", "ogr");
}

AttributeJoinDlg::~AttributeJoinDlg()
{
    delete ui;
}

void AttributeJoinDlg::run()
{
    //処理実行
    //ｓｈｐ作成
    QString filename = "D:\\テスト.shp";
    QString sss = "System";

    if (QFile(filename).exists())
        QgsVectorFileWriter::deleteShapeFile(filename);

    QgsFieldMap newFields;

    QgsVectorLayer *vlayer = vectorLayer("test");
    QgsFieldMap thisFields = vlayer->dataProvider()->fields();

    newFields = thisFields;

    QgsField field(ui->csvfilefield->itemText(0),QVariant::String);
    newFields.insert(thisFields.count(),field);

    QgsField field2(ui->csvfilefield->itemText(1),QVariant::String);
    newFields.insert(thisFields.count()+1,field2);

    QgsField field3(ui->csvfilefield->itemText(2),QVariant::Int);
    newFields.insert(thisFields.count()+2,field3);

    QGis::WkbType outputType = vlayer->dataProvider()->geometryType();
    const QgsCoordinateReferenceSystem crs = vlayer->crs();

    QgsVectorFileWriter *outfile = new QgsVectorFileWriter(filename, sss,
            newFields, outputType, &crs);


    QgsVectorDataProvider * provider = vlayer->dataProvider();
    QgsFieldMap fields = provider->fields();
    QgsAttributeList allAttrs = provider->attributeIndexes();

    provider->select(allAttrs);

    QgsFeature feat;

    QFile file( "D:\\テスト.csv" );
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            //qDebug()<< "cannot read file!";
    }

    file.readLine();

    //csvファイルをメモリにロード
    QMap<QString,QStringList> csvData;
    while (!file.atEnd()) {
        QString line = file.readLine();
        QStringList data = CSV::parseLine(line);
        csvData.insert(data.at(0),data);
    }


    while (provider->nextFeature(feat)) {

        QgsFeature newFeature;

        newFeature = feat;
        QStringList csvRec = csvData.value(feat.attributeMap().value(0).toString());
        if (csvRec.count() > 0) {
            newFeature.addAttribute(feat.attributeMap().count(),csvRec.at(0));
            newFeature.addAttribute(feat.attributeMap().count()+1,csvRec.at(1));
            newFeature.addAttribute(feat.attributeMap().count()+2,csvRec.at(2));
        }

        outfile->addFeature(newFeature);
    }


    delete outfile;
    file.close();

    mIface->addVectorLayer("D:\\テスト.shp", "テスト", "ogr");

}

QgsVectorLayer* AttributeJoinDlg::vectorLayer(QString layerName)
{
    QMap< QString, QgsMapLayer * > value = QgsMapLayerRegistry::instance()->mapLayers();

    QMap<QString, QgsMapLayer * >::const_iterator it = value.constBegin();
    for ( ; it != value.constEnd(); ++it)
    {
        QgsMapLayer *layer = it.value();
        if (layer->name() == layerName)
        {
            if ( layer == NULL || layer->type() != QgsMapLayer::VectorLayer )
            {
                return NULL;
            }
            QgsVectorLayer* vlayer = dynamic_cast<QgsVectorLayer*>( layer );
            return vlayer;
        }
    }

    return NULL;
}

