#-------------------------------------------------
#
# Project created by QtCreator 2011-12-20T08:22:54
#
#-------------------------------------------------

QT       += xml qt3support

TARGET = AttributeJoinFromCsv
TEMPLATE = lib

win32:LIBS += \
#"$$(QGISHOME)\lib\qgis_core.lib" \
#"$$(QGISHOME)\lib\qgis_gui.lib" \
"D:\OSGEO4W4\apps\qgis\lib\qgis_core.lib" \
"D:\OSGEO4W4\apps\qgis\lib\qgis_gui.lib" \
"$$(QGISHOME)\..\..\lib\geos_c_i.lib" \
"$$(QGISHOME)\..\gdal-17\lib\gdal_i.lib"


INCLUDEPATH += \
"$$(QGISHOME)\..\..\include" \
#"$$(QGISHOME)\include" \
"D:\OSGEO4W4\apps\qgis\include"


DEFINES += GUI_EXPORT= CORE_EXPORT=

SOURCES += AttributeJoinFromCsv.cpp \
    AttributeJoinDlg.cpp \
    csv.cpp \
    ErrorLog.cpp

HEADERS += AttributeJoinFromCsv.h \
    AttributeJoinDlg.h \
    csv.h \
    ErrorLog.h

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

OTHER_FILES += \
    icons/mmqgis_attribute_join.png

RESOURCES += \
    resource.qrc
