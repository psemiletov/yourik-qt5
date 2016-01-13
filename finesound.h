/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/

#ifndef FINESOUND_H
#define FINESOUND_H

//#include <QtCore>
#include <QObject>
#include <QString>
#include <QList>


#if !defined(__WIN32__)
#include "SDL/SDL_mixer.h"
#else
#include "SDL_mixer.h"
#endif


class CSample: public QObject
{
public:

  Mix_Chunk *chunk;
  int loop; //-1 = infinity, 0 = once, 1 - 2 loops, 2 - 3 loops, et cetera.

  CSample (const QString &fname, int loopval);
  bool load (const QString &fname);
};


class CSampleList: public QObject
{
public:

  QList <CSample *> samples;

  ~CSampleList();
  CSample *get(); //returns random sample from the list
};


class CMusic: public QObject
{
public:

  Mix_Music *music;
  int loop; //-1 = infinity, 0 = once, 1 - 2 loops, 2 - 3 loops, et cetera.

  CMusic (const QString &fname, int loopval);
  ~CMusic();
  bool load (const QString &fname);
};


class CSoundEngine: public QObject
{
public:

  CSoundEngine();
  ~CSoundEngine();

  void play_sample (CSample *s);
  void play_music (CMusic *m);
};


#endif // FINESOUND_H
