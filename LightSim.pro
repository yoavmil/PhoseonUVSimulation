#-------------------------------------------------
#
# Project created by QtCreator 2016-10-08T21:46:29
#
#-------------------------------------------------

QT += charts
QT += core gui widgets opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LightSim
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    widget3d.cpp \
    phoseonfe300.cpp \
    openglwindow.cpp

HEADERS  += mainwindow.h \
    widget3d.h \
    phoseonfe300.h \
    openglwindow.h

FORMS    += mainwindow.ui

#win32:CONFIG(release, debug|release): LIBS += -Lc:/code/assimp/code/release/ -lassimp-vc140-mt
#else:win32:CONFIG(debug, debug|release): LIBS += -Lc:/code/assimp/code/debug/ -lassimp-vc140-mt
DEFINES += GLM_FORCE_RADIANS
#INCLUDEPATH += C:/code/assimp-3.3.1/include
INCLUDEPATH += C:/code/glm

LIBS += opengl32.lib

RESOURCES = resources.qrc

DISTFILES += \
    surface.vert \
    surface.frag \
    lightsource.frag \
    lightsource.vert
