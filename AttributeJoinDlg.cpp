//include
#include "AttributeJoinDlg.h"
#include "ui_AttributeJoinDlg.h"
#include "csv.h"
#include "ErrorLog.h"

//QT include
#include <QFile>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

//QGIS include
#include <QgsVectorFileWriter.h>
#include <QgsVectorLayer.h>
#include <QgsMapLayerRegistry.h>
#include <QgsVectorDataProvider.h>
#include <QgsField.h>
#include <QGisInterface.h>

#include <math.h>

// 項目定義テーブル
#define ROW_HEIGHT                  25
#define TBL_FIELD_COUNT             5
#define TBL_CSV_FIELD_NAME          0
#define TBL_ATTR_FIELD_NAME         1
#define TBL_ATTR_FIELD_TYPE         2
#define TBL_ATTR_FIELD_LENGTH       3
#define TBL_ATTR_FIELD_PRECISION    4

// フィールドの型
#define FIELD_TYPE_STRING           1
#define FIELD_TYPE_INT              2
#define FIELD_TYPE_REAL             3

// フィールドの最大値
#define FIELD_TYPE_STRING_MAX       255
#define FIELD_TYPE_INT_MAX          10
#define FIELD_TYPE_REAL_MAX         20
#define FIELD_TYPE_PRECISION_MAX    5


#define FILE_ENCODING      tr("system")

AttributeJoinDlg::AttributeJoinDlg(QgisInterface* iface,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AttributeJoinDlg)
{
    ui->setupUi(this);
    mIface = iface;

    m_errLog = new ErrorLog();

    // ダイアログのコントロールボックスを設定
    setWindowFlags(Qt::Dialog | Qt::WindowSystemMenuHint | Qt::MSWindowsFixedSizeDialogHint);

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(joinProc()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    connect(ui->cmdSelectCsvFile, SIGNAL(clicked()), this, SLOT(selectCsvFile()));
    connect(ui->cmdSelectShapeFile, SIGNAL(clicked()), this, SLOT(selectShapeFile()));
    connect(ui->cmdSelectUnjoinedFile, SIGNAL(clicked()), this, SLOT(selectUnjoinedFile()));
    connect(ui->cboJoinLayer, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLayer(int)));
    connect(ui->radioComma, SIGNAL(clicked()), this, SLOT(changeCsvType()));
    connect(ui->radioTab, SIGNAL(clicked()), this, SLOT(changeCsvType()));

    initUi();
}

AttributeJoinDlg::~AttributeJoinDlg()
{
    delete m_errLog;
    delete ui;
}

void AttributeJoinDlg::initUi()
{
    ui->cboJoinLayer->clear();

    QMap<QString, QgsMapLayer*> value = QgsMapLayerRegistry::instance()->mapLayers();
    QMap<QString, QgsMapLayer * >::const_iterator ite;
    for(ite = value.constBegin(); ite != value.constEnd(); ite++) {
        QgsMapLayer *layer = ite.value();
        if(layer->type() == QgsMapLayer::VectorLayer) {
            ui->cboJoinLayer->addItem(layer->name(), layer->getLayerID());
        }
    }

    ui->cboJoinLayer->setCurrentIndex(-1);
}

void AttributeJoinDlg::changeLayer(int index)
{
    Q_UNUSED(index);

    ui->cboJoinLayerField->clear();

    QgsVectorLayer *vecLayer = getVectorLayerByName(ui->cboJoinLayer->currentText());
    if(vecLayer == NULL) return;

    QgsFieldMap attrMap = vecLayer->pendingFields();
    QgsFieldMap::iterator ite;
    QgsAttributeList attrList = vecLayer->pendingAllAttributesList();
    for(int i=0; i<attrList.count(); i++) {
        ite = attrMap.find(attrList.at(i));
        assert(ite != attrMap.end());
        QgsField field = *ite;
        ui->cboJoinLayerField->addItem(field.name());
    }
    ui->cboJoinLayerField->setCurrentIndex(-1);
}

// CSVファイル設定
void AttributeJoinDlg::selectCsvFile()
{
    QString csvFileName = QFileDialog::getOpenFileName( NULL, tr("ファイル選択"),
        tr(""), tr("CSVファイル (*.csv);;TSVファイル (*.tsv);;Textファイル (*.txt);;全て(*.*)"),
        NULL, QFileDialog::ReadOnly);
    if(csvFileName.isEmpty()) return;

    ui->txtCsvFileName->setText(csvFileName);

    loadCsvHeader(csvFileName);
}

void AttributeJoinDlg::loadCsvHeader(const QString& filename)
{
    QFile file( filename );
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::critical(NULL, tr("ファイルオープンエラー"), tr("ファイルのオープンに失敗しました\n%1").arg(filename));
        return;
    }

    // CSVからヘッダを取得してコンボに設定する
    QString lineBuf = file.readLine();
    QStringList header = CSV::parseLine(lineBuf, ui->radioTab->isChecked());
    ui->cboCsvFileField->clear();
    for(int i=0; i<header.count(); i++) {
        ui->cboCsvFileField->addItem(header.at(i));
    }
    ui->cboCsvFileField->setCurrentIndex(-1);

    // 項目テーブルを設定
    ui->tblCsvField->setRowCount(ui->cboCsvFileField->count());
    ui->tblCsvField->setColumnCount(TBL_FIELD_COUNT);
    QStringList headerLabels;
    headerLabels << tr("ファイル項目名") << tr("結合後の項目名") << tr("型") << tr("幅") << tr("精度");
    ui->tblCsvField->setHorizontalHeaderLabels(headerLabels);

    // ラベルやフレームでは setAutoFillBackground=true + setBackgroundRoleで背景色の変更ができるが
    // テキストボックスではなぜかできないので、パレットの色そのものを変更する
    QPalette	palette;
    palette = this->palette();
    palette.setColor(QPalette::Disabled, QPalette::Text, palette.color(QPalette::Active, QPalette::Text));

    for(int nrow=0; nrow<header.count(); nrow++) {

        QLineEdit *lineEdit;

        // 項目１ : 項目名
        lineEdit = new QLineEdit(header.at(nrow), ui->tblCsvField);
        lineEdit->setFrame(false);
        lineEdit->setEnabled(false);
        ui->tblCsvField->setCellWidget(nrow, TBL_CSV_FIELD_NAME, lineEdit);
        lineEdit->setPalette(palette);

        // 項目２ : 地物項目名
        lineEdit = new QLineEdit(header.at(nrow), ui->tblCsvField);
        lineEdit->setFrame(false);
        ui->tblCsvField->setCellWidget(nrow, TBL_ATTR_FIELD_NAME, lineEdit);

        // 項目３ : データ型
        QComboBox *cbTest = new QComboBox(this);
        cbTest->setEnabled(true);
        cbTest->clear();
        cbTest->addItem(tr("テキストデータ"), QVariant((int)FIELD_TYPE_STRING));
        cbTest->addItem(tr("整数値"), QVariant((int)FIELD_TYPE_INT));
        cbTest->addItem(tr("小数点付数値"), QVariant((int)FIELD_TYPE_REAL));
        cbTest->setCurrentIndex(0);
        ui->tblCsvField->setCellWidget(nrow, TBL_ATTR_FIELD_TYPE, cbTest);
        connect(cbTest, SIGNAL(currentIndexChanged(int)), this, SLOT(changeDataType(int)));

        // 項目４ : 桁数
        lineEdit = new QLineEdit("80", ui->tblCsvField);
        lineEdit->setFrame(false);
        ui->tblCsvField->setCellWidget(nrow, TBL_ATTR_FIELD_LENGTH, lineEdit);

        // 項目５ : 精度
        lineEdit = new QLineEdit("0", ui->tblCsvField);
        lineEdit->setFrame(false);
        QIntValidator *validator = new QIntValidator(0, FIELD_TYPE_PRECISION_MAX, lineEdit);
        lineEdit->setValidator(validator);
        ui->tblCsvField->setCellWidget(nrow, TBL_ATTR_FIELD_PRECISION, lineEdit);

        // 行幅を設定
        ui->tblCsvField->setRowHeight(nrow, ROW_HEIGHT);
    }

    changeDataType(0);

    //	fit of column size at contents.
    ui->tblCsvField->resizeColumnsToContents();

    // 幅、精度はフィットサイズでは広すぎるので、狭くしておく
    ui->tblCsvField->setColumnWidth(TBL_ATTR_FIELD_LENGTH, 40);
    ui->tblCsvField->setColumnWidth(TBL_ATTR_FIELD_PRECISION, 40);

    file.close();
}

void AttributeJoinDlg::changeCsvType()
{
    // ロードしなおす
    QString filename = ui->txtCsvFileName->text().trimmed();
    if(filename.isEmpty()) return;

    loadCsvHeader(filename);
}

void AttributeJoinDlg::changeDataType(int index)
{
    Q_UNUSED(index);

    int             nrow, length;
    QComboBox       *comboBox;
    QLineEdit       *editLength, *editPrecision;
    QIntValidator   *validator;

    for(nrow=0; nrow<ui->tblCsvField->rowCount(); nrow++) {

        comboBox = dynamic_cast <QComboBox*>(ui->tblCsvField->cellWidget(nrow, TBL_ATTR_FIELD_TYPE));
        editLength = dynamic_cast <QLineEdit*>(ui->tblCsvField->cellWidget(nrow, TBL_ATTR_FIELD_LENGTH));
        editPrecision = dynamic_cast <QLineEdit*>(ui->tblCsvField->cellWidget(nrow, TBL_ATTR_FIELD_PRECISION));

        switch(comboBox->itemData(comboBox->currentIndex(), Qt::UserRole).toInt()) {
            case FIELD_TYPE_STRING:
                editPrecision->setEnabled(false);
                validator = new QIntValidator(0, FIELD_TYPE_STRING_MAX, editLength);
                editLength->setValidator(validator);
                break;
            case FIELD_TYPE_INT:
                editPrecision->setEnabled(false);
                length = editLength->text().toInt();
                if(length < 0 || length > FIELD_TYPE_INT_MAX) {
                    editLength->setText(tr("%1").arg(FIELD_TYPE_INT_MAX));
                }
                validator = new QIntValidator(0, FIELD_TYPE_INT_MAX, editLength);
                editLength->setValidator(validator);
                break;
            case FIELD_TYPE_REAL:
                editPrecision->setEnabled(true);
                length = editLength->text().toInt();
                if(length < 0 || length > FIELD_TYPE_REAL_MAX) {
                    editLength->setText(tr("%1").arg(FIELD_TYPE_REAL_MAX));
                }
                validator = new QIntValidator(0, FIELD_TYPE_REAL_MAX, editLength);
                editLength->setValidator(validator);
                break;
        }


    }
}

// shapeファイル設定
void AttributeJoinDlg::selectShapeFile()
{
    QString shapeFileName = QFileDialog::getSaveFileName( NULL,
                                                        tr("shapeファイル選択"),
                                                        tr(""), tr("*.shp"), NULL, 0);
    if(shapeFileName.isEmpty()) return;

    ui->txtShapeFileName->setText(shapeFileName);
}

// 未結合のCSVファイル設定
void AttributeJoinDlg::selectUnjoinedFile()
{
    QString unjoinedFileName = QFileDialog::getSaveFileName( NULL,
                                                        tr("CSVファイル選択"),
                                                        tr(""), tr("*.csv"), NULL, 0);
    if(unjoinedFileName.isEmpty()) return;

    ui->txtUnjoinedFileName->setText(unjoinedFileName);
}

// 結合前の準備処理
bool AttributeJoinDlg::preJoinProc()
{
    QString         layerName;
    bool            flagOk;

    // CSVファイル名
    m_csvFileName = ui->txtCsvFileName->text().trimmed();
    if(m_csvFileName.isEmpty()) {
        QMessageBox::critical(NULL, tr("入力エラー"), tr("テキストファイルを指定してください"));
        ui->txtCsvFileName->setFocus();
        return false;
    }

    // Shapeファイル名
    m_shapeFileName = ui->txtShapeFileName->text().trimmed();
    if(m_shapeFileName.isEmpty()) {
        QMessageBox::critical(NULL, tr("入力エラー"), tr("Shapeファイルを指定してください"));
        ui->txtShapeFileName->setFocus();
        return false;
    }

    // エラーファイル名
    m_unjoinedFileName = ui->txtUnjoinedFileName->text().trimmed();
    if(m_unjoinedFileName.isEmpty()) {
        QMessageBox::critical(NULL, tr("入力エラー"), tr("エラーリストファイルを指定してください"));
        ui->txtUnjoinedFileName->setFocus();
        return false;
    }

    // ファイル名のチェック
    if( m_csvFileName == m_shapeFileName ||
        m_csvFileName == m_unjoinedFileName ||
        m_shapeFileName == m_unjoinedFileName) {
        QMessageBox::critical(NULL, tr("入力エラー"), tr("テキストファイル、出力Shape、結合できなかったCSVレコードリストのいずれかに同じファイル名が指定されています"));
        return false;
    }

    // エラーログのオープン
    if(!m_errLog->open(m_unjoinedFileName)) return false;

    // CSVマッチングフィールド
    m_csvKeyName = ui->cboCsvFileField->currentText();
    if(m_csvKeyName.isEmpty()) {
        QMessageBox::critical(NULL, tr("入力エラー"), tr("テキストファイルのマッチングフィールドを指定してください"));
        ui->cboCsvFileField->setFocus();
        return false;
    }

    // レイヤ名
    layerName = ui->cboJoinLayer->currentText();
    if(layerName.isEmpty()) {
        QMessageBox::critical(NULL, tr("入力エラー"), tr("レイヤを指定してください"));
        ui->cboJoinLayer->setFocus();
        return false;
    }

    // レイヤマッチングフィールド
    m_layerKeyName = ui->cboJoinLayerField->currentText();
    if(m_layerKeyName.isEmpty()) {
        QMessageBox::critical(NULL, tr("入力エラー"), tr("レイヤのマッチングフィールドを指定してください"));
        ui->cboJoinLayerField->setFocus();
        return false;
    }

    // 既存のレイヤの属性項目に新しい属性項目を追加して新しい属性リストを作成する
    m_vecLayer = getVectorLayerByName(layerName);
    QgsVectorDataProvider *vecProvider = m_vecLayer->dataProvider();
    m_orgFieldMap = vecProvider->fields();
    m_newFieldMap = m_orgFieldMap;
    QgsFieldMap::iterator ite;
    QgsField newField;

    //フィールド名重複の際の連番
    int tempFieldCount = 0;


    for(int nrow=0; nrow<ui->tblCsvField->rowCount(); nrow++) {

        QString         fieldName;
        int             fieldLengthMax;
        QLineEdit       *lineEditName, *lineEditLength, *lineEditPrecision;
        QComboBox       *cboFieldType;

        // フィールド名の取得
        lineEditName = dynamic_cast <QLineEdit*>(ui->tblCsvField->cellWidget(nrow, TBL_ATTR_FIELD_NAME));
        fieldName = lineEditName->text().trimmed();
        if(fieldName.isEmpty()) {
            QMessageBox::critical(NULL, tr("入力エラー"), tr("項目名を入力してください"));
            lineEditName->setFocus();
            return false;
        }

        // 属性項目名が重複していないかチェック
        if(isExistField(m_newFieldMap, fieldName)) {
            //重複している場合には別名を指定する
            fieldName = "_dat" + QString("%1").arg(QString::number(tempFieldCount), 4, QChar('0'));
            ++tempFieldCount;
            lineEditName->setText(fieldName);

            //QMessageBox::critical(NULL, tr("入力エラー"), tr("この項目名はすでに存在します"));
            //lineEditName->setFocus();
            //return false;
        }

        // フィールドに名前を設定
        newField.setName(fieldName);

        // タイプの取得
        cboFieldType = dynamic_cast <QComboBox*>(ui->tblCsvField->cellWidget(nrow, TBL_ATTR_FIELD_TYPE));
        switch(cboFieldType->itemData(cboFieldType->currentIndex(), Qt::UserRole).toInt()) {
            case FIELD_TYPE_STRING:
                newField.setType(QVariant::String);
                fieldLengthMax = FIELD_TYPE_STRING_MAX;
                break;
            case FIELD_TYPE_INT:
                newField.setType(QVariant::Int);
                fieldLengthMax = FIELD_TYPE_INT_MAX;
                break;

            case FIELD_TYPE_REAL:
                newField.setType(QVariant::Double);
                fieldLengthMax = FIELD_TYPE_REAL_MAX;
                break;
        }

        // 項目長の取得
        lineEditLength = dynamic_cast <QLineEdit*>(ui->tblCsvField->cellWidget(nrow, TBL_ATTR_FIELD_LENGTH));
        newField.setLength(lineEditLength->text().toInt(&flagOk));
        if(!flagOk || newField.length() <= 0 || newField.length() > fieldLengthMax) {
            QMessageBox::critical(NULL, tr("入力エラー"), tr("適切なフィールドの長さを指定してください。[1 ～ %2]").arg(fieldLengthMax));
            lineEditLength->setFocus();
            return false;
        }

        // 項目長の取得
        lineEditPrecision = dynamic_cast <QLineEdit*>(ui->tblCsvField->cellWidget(nrow, TBL_ATTR_FIELD_PRECISION));
        if(newField.type() != QVariant::Double) {
            newField.setPrecision(0);
        } else {

            newField.setPrecision(lineEditPrecision->text().toInt(&flagOk));
            if(!flagOk || newField.precision() < 0 || newField.precision() > FIELD_TYPE_PRECISION_MAX) {
                QMessageBox::critical(NULL, tr("入力エラー"), tr("適切なフィールドの長さを指定してください。[0 ～ %2]").arg(FIELD_TYPE_PRECISION_MAX));
                lineEditPrecision->setFocus();
                return false;
            }
        }

        // マッチングフィールドになっている場合は、レイヤのマッチングフィールドに合わせる
//        if(fieldName == m_csvKeyName) {
        if(nrow == ui->cboCsvFileField->currentIndex()) {
            m_keyField = getField(m_newFieldMap, m_layerKeyName);
            if( m_keyField.type() != newField.type() ||
                m_keyField.length() != newField.length() ||
                m_keyField.precision() != newField.precision()) {

                newField.setType(m_keyField.type());
                newField.setLength(m_keyField.length());
                newField.setPrecision(m_keyField.precision());

                // 変更内容を画面に反映
                switch(newField.type()) {

                    case QVariant::String:
                        cboFieldType->setCurrentIndex(0);
                        break;

                    case QVariant::Int:
                    case QVariant::UInt:
                        cboFieldType->setCurrentIndex(1);
                        break;

                    case QVariant::Double:
                        cboFieldType->setCurrentIndex(2);
                        break;
                }

                lineEditLength->setText(tr("%1").arg(newField.length()));
                lineEditPrecision->setText(tr("%1").arg(newField.precision()));

                QMessageBox::information(this, tr(""), tr("マッチングフィールドの型が合わないため、レイヤのフィールドに合わせて変更しました"));
                this->repaint();
            }
        }

        // 現在のフィールドリストに追加
        m_newFieldMap.insert(m_newFieldMap.count(), newField);
    }

    return true;
}

// CSVを全件読み込む
bool AttributeJoinDlg::loadCsvData(const QString& filename)
{
    QFile csvFile(filename);
    if (!csvFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        QMessageBox::critical(NULL, tr("ファイルオープンエラー"), tr("テキストファイルのオープンに失敗しました\n%1").arg(filename));
        return false;
    }

    int keyIndex = ui->cboCsvFileField->currentIndex();

    // csvファイルを読み込み、順番に処理する
    long    recCount = 0;

    //csvファイルをメモリにロード
    while(!csvFile.atEnd()) {

        recCount++;

        // CSVを１行読み込んで項目に分割（末尾の'\n'を削除する）
        QByteArray barray = csvFile.readLine();
        barray.remove(barray.count() - 1, 1);

        // １行目はヘッダ行なので読み飛ばし
        if(recCount == 1) continue;

        QString lineBuf = barray;

        // 現状では文字列区切りなし＝ダブルクォーテーションは文字として認識
        // スペースが入るとおかしくなる
        QStringList data = CSV::parseLine(lineBuf, ui->radioTab->isChecked());

        // 項目が足りないときは補完する
        if(data.count() < ui->tblCsvField->rowCount()) {
            for(int i=data.count(); i<ui->tblCsvField->rowCount(); i++) {
                data.append(tr(""));
            }
        }

        m_csvData.insert(data.at(keyIndex), data);
        m_csvKey.append(data.at(keyIndex));
    }

    csvFile.close();

    return true;
}

QString AttributeJoinDlg::createFieldValueString(const QgsField& field, const QVariant& var)
{
    bool        flagOk;
    QString     retStr;

    switch(field.type()) {
        case QVariant::String:
        {
            retStr = var.toString();
            break;
        }
        case QVariant::Int:
        {
            int item = var.toInt(&flagOk);
            if(!flagOk) break;
            retStr.setNum(item);
            break;
        }
        case QVariant::UInt:
        {
            uint item = var.toUInt(&flagOk);
            if(!flagOk) break;
            retStr.setNum(item);
            break;
        }
        case QVariant::Double:
        {
            double item = var.toDouble(&flagOk);
            if(!flagOk) break;
            retStr.setNum(item, 'f', field.precision());
            break;
        }
    }

    return retStr;
}

// 属性との結合処理
void AttributeJoinDlg::joinProc()
{
    QgsFeature orgFeature;

    // カーソルをBUSYに変更
    QCursor orgCursor = this->cursor();
    setCursor(Qt::BusyCursor);

    // 結合前処理
    if(!preJoinProc()) {
        setCursor(orgCursor);
        return;
    }

    // ここまでで新しく作成される地物の属性フィールドマップが作成されている

    // CSVの読み込み
    if(!loadCsvData(m_csvFileName)) {
        setCursor(orgCursor);
        return;
    }

    // shapeが既にあるなら一旦消す
    if (QFile(m_shapeFileName).exists()) {
        QgsVectorFileWriter::deleteShapeFile(m_shapeFileName);
    }

    QGis::WkbType geometryType = m_vecLayer->wkbType();
    const QgsCoordinateReferenceSystem crs = m_vecLayer->crs();

    QgsVectorFileWriter *shapeFile = new QgsVectorFileWriter(m_shapeFileName, FILE_ENCODING, m_newFieldMap, geometryType, &crs);

    // 対象レイヤの地物を全件読んでマッチングする
    // 地物内のキー項目のインデックスを取得
    int layerKeyIndex = m_vecLayer->fieldNameIndex(m_layerKeyName);
    m_vecLayer->select(m_vecLayer->pendingAllAttributesList(), QgsRectangle(), true, false);

    QList<QVariant> csvRec;

    QString logInfo;
    m_errLog->open(m_unjoinedFileName);

    while(m_vecLayer->nextFeature(orgFeature)) {

        QgsFeature newFeature = orgFeature;

        // 地物のキー項目の値を取得
        QgsAttributeMap attrMap = newFeature.attributeMap();
        QgsAttributeMap::iterator iteAttr = attrMap.find(layerKeyIndex);
        assert(iteAttr != attrMap.end());
        QVariant featureKey = iteAttr.value();
//        QString keyVar = createFieldValueString(m_keyField, iteAttr.value());

        //CSVマップから検索
        QMap< QString, QStringList >::iterator iteCsvRec;
//        iteCsvRec = m_csvData.find(keyVar);
        bool bFound = false;
        QString foundKey;
        for(iteCsvRec = m_csvData.begin(); iteCsvRec != m_csvData.end(); iteCsvRec++) {
            foundKey = iteCsvRec.key();
            QVariant csvKey = createFieldValue(m_keyField, foundKey);
            switch(csvKey.type()) {
                case QVariant::String:
                    if(csvKey.toString() == featureKey.toString()) {
                        bFound = true;
                    }
                    break;
                case QVariant::Int:
                    if(csvKey.toInt() == featureKey.toInt()) {
                        bFound = true;
                    }
                    break;
                case QVariant::UInt:
                    if(csvKey.toUInt() == featureKey.toUInt()) {
                        bFound = true;
                    }
                    break;
                case QVariant::Double:
                    if(csvKey.toDouble() == featureKey.toDouble()) {
                        bFound = true;
                    }
                    break;
            }
            if(bFound) break;
        }

//        if(iteCsvRec == m_csvData.end()) {
        if(!bFound) {
            // そのまま出力
        } else {
            //型変換
            QStringList data = iteCsvRec.value();
            csvRec.clear();
            for(int i=0; i<data.count(); i++) {
                QgsFieldMap::iterator ite = m_newFieldMap.find(m_orgFieldMap.count() + i);
                QVariant var = createFieldValue(*ite, data.at(i));
                if(var.type() == QVariant::Invalid) {
                    //エラー
                    QString errcsv = data.join("\",\"");
                    logInfo = tr("\"フィールドの型変換に失敗 項目(%1)\",\"%2\"\n").arg(i+1).arg(errcsv);
                    m_errLog->outLog(logInfo);
                    break;
                }
                csvRec.append(var);
            }

            // 非参照リストから消す
            for(int i=0; i<m_csvKey.count(); i++) {
                if(m_csvKey.at(i) == foundKey) {
                    m_csvKey.removeAt(i);
                    break;
                }
            }

            // CSVの内容を追加
            for(int i=0; i<csvRec.count(); i++) {
                newFeature.addAttribute(newFeature.attributeMap().count(), csvRec.at(i));
            }
        }
        shapeFile->addFeature(newFeature);
    }

    delete shapeFile;

    //非参照リストを出力
    for(int i=0; i<m_csvKey.count(); i++) {
        //CSVマップから検索
        QMap< QString, QStringList >::iterator iteCsvRec;
        iteCsvRec = m_csvData.find(m_csvKey.at(i));
        if(!(iteCsvRec == m_csvData.end())) {
            QStringList data = iteCsvRec.value();
            QString errcsv = data.join("\",\"");
            logInfo = tr("\"不一致レコード\",\"%1\"\n").arg(errcsv);
            m_errLog->outLog(logInfo);
        }
    }

    m_errLog->close();

    if(ui->chkAddToProject->isChecked()) {
        int pos;
        for(pos=m_shapeFileName.length(); pos>=0; pos--) {
            if(m_shapeFileName.at(pos) == '\\' || m_shapeFileName.at(pos) == '/') {
                pos++;
                break;
            }
        }

        QString addLayerName = m_shapeFileName.mid(pos);
        mIface->addVectorLayer(m_shapeFileName, addLayerName, tr("ogr"));
    }

    QMessageBox::information(this, tr(""), tr("処理が終了しました"));

    // カーソルを元に戻す
    setCursor(orgCursor);
}

QgsVectorLayer* AttributeJoinDlg::getVectorLayerByName(QString layerName)
{
    QMap<QString, QgsMapLayer*> value = QgsMapLayerRegistry::instance()->mapLayers();
    QMap<QString, QgsMapLayer*>::const_iterator ite;

    for(ite = value.constBegin(); ite != value.constEnd(); ite++) {
        QgsMapLayer *layer = ite.value();
        if(layer->name() == layerName) {
            if(layer->type() != QgsMapLayer::VectorLayer) {
                // ベクタレイヤでない
                return NULL;
            }

            QgsVectorLayer* vecLayer = dynamic_cast<QgsVectorLayer*>(layer);
            return vecLayer;
        }
    }

    // 見つからない
    return NULL;
}


bool AttributeJoinDlg::isExistField(QgsFieldMap fieldMap, const QString& fieldName)
{
    QgsFieldMap::iterator ite;


    for(ite = fieldMap.begin(); ite != fieldMap.end(); ite++) {
        if((*ite).name() == fieldName) {
            return true;
        }
    }

    return false;
}

QgsField AttributeJoinDlg::getField(QgsFieldMap fieldMap, const QString& fieldName)
{
    QgsFieldMap::iterator ite;


    for(ite = fieldMap.begin(); ite != fieldMap.end(); ite++) {
        if((*ite).name() == fieldName) {
            return *ite;
        }
    }

    return QgsField();
}

QVariant AttributeJoinDlg::createFieldValue(const QgsField& field, QString text)
{
    bool        flagOk;
    QVariant    var;
    QString     sItem;


    // 文字型は別処理
    if(field.type() == QVariant::String) {
        return QString(text);
    }

    // 数値型でCSVの項目が省略された場合の対応
    if(text.isEmpty()) {
        sItem = "0";
    } else {
        sItem = text;
    }

    // 以下、数値系で何がしかの文字列がある場合
    var.clear();

    switch(field.type()) {

        case QVariant::Int:
        {
            // 1.6とかも切り捨てて1として正常に受け付けるようにする
            double dItem = sItem.toDouble(&flagOk);
            if(flagOk && (double)INT_MIN <= dItem && dItem <= (double)INT_MAX) {
                var = (int)dItem;
//QMessageBox::information(this, tr("数値"), tr("%1").arg(item));
            }
            break;
        }
        case QVariant::UInt:
        {
            // 1.6とかも切り捨てて1として正常に受け付けるようにする
            double dItem = sItem.toDouble(&flagOk);
            if(flagOk && 0.0 <= dItem && dItem <= (double)UINT_MAX) {
                var = (uint)dItem;
//QMessageBox::information(this, tr("数値"), tr("%1").arg(item));
            }
            break;
        }
        case QVariant::Double:
        {
            double dItem = sItem.toDouble(&flagOk);
            if(flagOk) {
                // 有効数字に合わせてチェックする
                double dLimit = powf(10, field.length() - field.precision());
                if(-dLimit < dItem && dItem < dLimit) {
                    double ds = powf(10.0, field.precision());
                    double imItem = floor(dItem);
                    double reItem = floor((dItem - imItem) * ds) / ds;
                    var = imItem + reItem;
//QMessageBox::information(this, tr("数値"), tr("%1").arg(item));
                }
            }
            break;
        }
    }

    return var;
}
