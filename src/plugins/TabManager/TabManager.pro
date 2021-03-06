TARGET = $$qtLibraryTarget(TabManager)
# OS/2 allows only 8 chars in TARGET
os2: TARGET = TabManPl

SOURCES += tabmanagerplugin.cpp \
    tabmanagerwidget.cpp \
    tabmanagerwidgetcontroller.cpp \
    tabmanagersettings.cpp

HEADERS += tabmanagerplugin.h \
    tabmanagerwidget.h \
    tabmanagerwidgetcontroller.h \
    tabmanagersettings.h

RESOURCES += tabmanagerplugin.qrc

FORMS += \
    tabmanagerwidget.ui \
    tabmanagersettings.ui

TRANSLATIONS = \
    translations/fa_IR.ts

include(tldextractor/tldextractor.pri)

PLUGIN_DIR = $$PWD
srcdir = $$(QUPZILLA_SRCDIR)
equals(srcdir, "") {
    include(../../plugins.pri)
}
else {
    include($$srcdir/src/plugins.pri)
}
