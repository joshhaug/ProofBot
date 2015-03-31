QT += widgets
QT += printsupport

macx {
     QMAKE_INFO_PLIST = Info.plist
     ICON = application.icns
}

HEADERS       = \
    mainwindow.h \
    highlighter.h \
    completions.h
SOURCES       = main.cpp \
                mainwindow.cpp \
    highlighter.cpp \
    completions.cpp
TARGET = ProofBot
#! [0]
RESOURCES     = application.qrc
#! [0]

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/mainwindows/application
INSTALLS += target

DISTFILES += \
    readme.txt \
    theorem_list.txt
