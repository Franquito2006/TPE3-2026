QT += widgets network printsupport

CONFIG += c++17 -Wa,-mbig-obj

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

LIBS += -L"C:\qcp_lib" -l:qcustomplotd2.dll
INCLUDEPATH += "C:\qcp_lib"
LIBS += -L"C:\muparser_lib" -l:MuParserCreationDLL_2024.dll
INCLUDEPATH += "C:\muparser_lib"
