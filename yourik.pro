VERSION = 2.0.0
QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4) {
       QT += widgets
   } else {
#QT += blah blah blah
   }

TEMPLATE = app

isEmpty(PREFIX):PREFIX = /usr/local
BINDIR = $$PREFIX/bin
DATADIR = $$PREFIX

TARGET = yourik
target.path = $$BINDIR


SOURCES += main.cpp\
           cmainwindow.cpp \
           game_elements.cpp \
           finesound.cpp \
           utils.cpp

INSTALLS += target
#RESOURCES += yourik.qrc

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
             snd/*


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


#INCLUDEPATH += `sdl-config --cflags`
#LIBS += `sdl2-config --libs`
#QMAKE_LIBS += `sdl2-config --libs` 
