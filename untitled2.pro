QT += widgets

CONFIG += c++17

SOURCES += \
    envmanager.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    envmanager.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32 {
    exists($$PWD/python-3.10.11-amd64.exe) {
        copypython.commands = $(COPY_FILE) \"$$shell_path($$PWD/python-3.10.11-amd64.exe)\" \"$$shell_path($$OUT_PWD)\"
        QMAKE_EXTRA_TARGETS += copypython
        POST_TARGETDEPS += copypython
    }
    exists($$PWD/proxy_relay.py) {
        copyrelay.commands = $(COPY_FILE) \"$$shell_path($$PWD/proxy_relay.py)\" \"$$shell_path($$OUT_PWD)\"
        QMAKE_EXTRA_TARGETS += copyrelay
        POST_TARGETDEPS += copyrelay
    }
}