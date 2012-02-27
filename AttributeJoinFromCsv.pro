#-------------------------------------------------
#
# Project created by QtCreator 2011-12-20T08:22:54
#
#-------------------------------------------------
QGIS_DIR = C:\Program Files\qgis1.6.0
OSGEO4_W = D:\OSGEO4W

QT       += xml qt3support

TARGET = AttributeJoinFromCsv
TEMPLATE = lib


win32:LIBS += \
"$$QGIS_DIR\lib\qgis_core.lib" \
"$$QGIS_DIR\lib\qgis_gui.lib" \
"$$OSGEO4_W\lib\geos_c_i.lib" \
"$$OSGEO4_W\lib\gdal_i.lib" \

INCLUDEPATH += \
"$$OSGEO4_W\include" \
"$$QGIS_DIR\include" \

DEFINES += GUI_EXPORT= CORE_EXPORT=

SOURCES += AttributeJoinFromCsv.cpp \
    AttributeJoinDlg.cpp \
    csv.cpp

HEADERS += AttributeJoinFromCsv.h \
    AttributeJoinDlg.h \
    csv.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE55FC909
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = AttributeJoinFromCsv.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

FORMS += \
    AttributeJoinDlg.ui
