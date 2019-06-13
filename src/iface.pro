#-------------------------------------------------
#
# Project created by QtCreator 2018-09-19T12:08:03
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = iface
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        iface.cpp \
        fastq2bam.cpp \
    hpgmethyl_worker.cpp \
    bam2csv.cpp \
    hpghmapper_worker.cpp

HEADERS += \
        iface.h \
        fastq2bam.h \
    hpgmethyl_worker.h \
    bam2csv.h \
    hpghmapper_worker.h

FORMS += \
        iface.ui \
        fastq2bam.ui \
    bam2csv.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    image/images.qrc

DISTFILES +=
