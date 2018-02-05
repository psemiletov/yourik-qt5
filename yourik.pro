VERSION = 2.1.2
QT += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4) {
       QT += widgets
   } else {
#QT += blah blah blah
   }

TEMPLATE = app

win32:{

isEmpty(PREFIX) {
PREFIX = /usr/local/bin
}

TARGET = bin/eko
target.path = $$PREFIX
}


unix:{

isEmpty(PREFIX) {
PREFIX = /usr/local
}

message(UNIX HERE)

PREFIX = $$replace(PREFIX, bin,)

TARGET = bin/yourik
target.path = $$PREFIX/bin
desktop.path=$$PREFIX/share/applications
desktop.files=desktop/yourik.desktop

icon64.path = $$PREFIX/share/icons/hicolor/64x64/apps/
icon64.files += icons/yourik.png

INSTALLS += desktop icon64
}


DATADIR = $$PREFIX

SOURCES += main.cpp\
           cmainwindow.cpp \
           game_elements.cpp \
           finesound.cpp \
           utils.cpp

INSTALLS += target

HEADERS  += cmainwindow.h \
            game_elements.h \
            finesound.h \
            utils.h

DISTFILES += COPYING \
             TODO \
             README \
             INSTALL \
             AUTHORS \
             HACKING \
             ChangeLog \
             pix/* \
             titles/* \
             snd/* \
             icons/* \
             desktop/*


win32:{
       QMAKE_CXXFLAGS += -mwindows
       LIBS += -lSDLmain
       LIBS += -lSDL.dll
       LIBS += -lSDL_mixer
       INCLUDEPATH += F:/Qt/2010.05/mingw/include/SDL
}



unix: {
       PKGCONFIG += SDL2 \
                    SDL2_mixer


       LIBS += -lSDL2
       LIBS += -lSDL2_mixer

       snd.path = $$PREFIX/share/yourik/snd
       snd.files += snd/*
       INSTALLS += snd

       pix.path = $$PREFIX/share/yourik/pix
       pix.files += pix/*
       INSTALLS += pix

       titles.path = $$PREFIX/share/yourik/titles
       titles.files += titles/*
       INSTALLS += titles

#     QMAKE_CXXFLAGS += -DMYDATADIR=\"\"$$PREFIX/share/yourik/snd\"\"
       DEFINES += 'SNDDIR=\\\"$$PREFIX/share/yourik/snd/\\\"'
       DEFINES += 'PIXDIR=\\\"$$PREFIX/share/yourik/pix/\\\"'
       DEFINES += 'TITLESDIR=\\\"$$PREFIX/share/yourik/titles/\\\"'
}

