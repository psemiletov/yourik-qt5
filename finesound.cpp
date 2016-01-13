/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/

#include <QDebug>

#include <SDL2/SDL.h>
#include "SDL2/SDL_mixer.h"

#include "finesound.h"


CSoundEngine::CSoundEngine()
{
//  int flags = MIX_INIT_OGG | MIX_INIT_MOD;
#if (SDL_MIXER_MAJOR_VERSION == 1) && (SDL_MIXER_MINOR_VERSION >= 2) && (SDL_MIXER_PATCHLEVEL >= 10)

  qDebug() << "Mix_Init";

  int flags = MIX_INIT_OGG;
  int initted = Mix_Init (flags);
  if ((initted & flags) != flags)
     {
      qDebug() << "Mix_Init: Failed to init required ogg support!" ;
      qDebug() << "Mix_Init: " << Mix_GetError();
     }
#endif

		Mix_OpenAudio (48000, MIX_DEFAULT_FORMAT, 2, 4096);
		Mix_AllocateChannels (32);
}


CSoundEngine::~CSoundEngine()
{
  Mix_CloseAudio();

#if (SDL_MIXER_MAJOR_VERSION == 1) && (SDL_MIXER_MINOR_VERSION >= 2) && (SDL_MIXER_PATCHLEVEL >= 10)
  Mix_Quit();

  qDebug() << "Mix_Quit";

#endif
}


void CSoundEngine::play_sample (CSample *s)
{
  if (! s)
     return;

  Mix_PlayChannel (-1, s->chunk, s->loop);
}


bool CSample::load (const QString &fname)
{
  bool result = true;

  chunk = Mix_LoadWAV (fname.toLatin1().data());

  if (! chunk)
     {
      qDebug() << "cannot load " << fname;
      result = false;
     }

  return result;
}


CSample::CSample (const QString &fname, int loopval)
{
  load (fname);
  loop = loopval;
}


CMusic::CMusic (const QString &fname, int loopval)
{
  load (fname);
  loop = loopval;
}


bool CMusic::load (const QString &fname)
{
  bool result = true;

  music = Mix_LoadMUS (fname.toLatin1().data());

  if (! music)
     {
      qDebug() << "cannot load " << fname;
      result = false;
     }

  return result;
}


CMusic::~CMusic()
{
  Mix_FreeMusic (music);
}


void CSoundEngine::play_music (CMusic *m)
{
  if (! m)
     return;

  Mix_HaltMusic();
  Mix_PlayMusic (m->music, m->loop);
}


CSampleList::~CSampleList()
{
  foreach (CSample *s, samples)
          delete s;
}


CSample *CSampleList::get()
{
  int i = qrand() % samples.count();
  return samples.at (i);
}
