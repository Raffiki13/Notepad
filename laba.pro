QT += widgets
requires(qtConfig(filedialog))

HEADERS       = mainwindow.h \
    highlighter.h
SOURCES       = main.cpp \
                highlighter.cpp \
                mainwindow.cpp
#! [0]
RESOURCES     = application.qrc
#! [0]

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/mainwindows/application
INSTALLS += target
