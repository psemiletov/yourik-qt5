/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/


#ifndef GAME_ELEMENTS_H
#define GAME_ELEMENTS_H


#include <QObject>
#include <QList>
#include <QImage>


#include "finesound.h"


#define RES_X 800
#define RES_Y 600

#define SOURCE_RES_X 800
#define SOURCE_RES_Y 600
//#define DEST_RES_X 1024
//#define DEST_RES_Y 768


#define CYC_PER_SECOND 50
#define MAX_SPRITES 256
#define MAX_SOUNDS 256


enum
{
  SPR_VIVI01_DIR_LEFT = 0,
  SPR_VIVI01_DIR_RIGHT,
  SPR_VIVI01_DIR_UP,
  SPR_VIVI01_DIR_DOWN,
  SPR_VIVI01_STAY_AT_PLACE,

  SPR_VIVI02_DIR_LEFT,
  SPR_VIVI02_DIR_RIGHT,
  SPR_VIVI02_DIR_UP,
  SPR_VIVI02_DIR_DOWN,
  SPR_VIVI02_STAY_AT_PLACE,

  SPR_ENEMY01_DIR_LEFT,
  SPR_ENEMY01_DIR_RIGHT,
  SPR_ENEMY01_DIR_UP,
  SPR_ENEMY01_DIR_DOWN,
  SPR_ENEMY01_STAY_AT_PLACE,

  SPR_ENEMY02_DIR_LEFT,
  SPR_ENEMY02_DIR_RIGHT,
  SPR_ENEMY02_DIR_UP,
  SPR_ENEMY02_DIR_DOWN,
  SPR_ENEMY02_STAY_AT_PLACE,

  SPR_ANIMAL01,
  SPR_ANIMAL02,
  SPR_ANIMAL03,

  SPR_CARROT_DIR_LEFT,
  SPR_CARROT_DIR_RIGHT,
  SPR_CARROT_DIR_UP,
  SPR_CARROT_DIR_DOWN,

  SPR_HERO_DIR_LEFT,
  SPR_HERO_DIR_RIGHT,
  SPR_HERO_DIR_UP,
  SPR_HERO_DIR_DOWN,
  SPR_HERO_STAY_AT_PLACE,

  SPR_HERO_ROCKET_DIR_LEFT,
  SPR_HERO_ROCKET_DIR_RIGHT,
  SPR_HERO_ROCKET_DIR_UP,
  SPR_HERO_ROCKET_DIR_DOWN,
  SPR_HERO_ROCKET_STAY_AT_PLACE,

  SPR_STAR_STAY_AT_PLACE,
  SPR_ROCKET_STAY_AT_PLACE,

  SPR_BEAM_DIR_LEFT,
  SPR_BEAM_DIR_RIGHT,
  SPR_BEAM_DIR_UP,
  SPR_BEAM_DIR_DOWN,
  SPR_BEAM_STAY_AT_PLACE,

  SPR_KNIFE_DIR_DOWN,
  SPR_WRENCH_DIR_DOWN,

  SPR_ENEMY_BALL_WEAPON_DIR_LEFT,
  SPR_ENEMY_BALL_WEAPON_DIR_RIGHT,
  SPR_ENEMY_BALL_WEAPON_DIR_UP,
  SPR_ENEMY_BALL_WEAPON_DIR_DOWN,
  SPR_ENEMY_BALL_WEAPON_STAY_AT_PLACE,

  SPR_NUKE_STAY_AT_PLACE,
  SPR_KATAFALK_STAY_AT_PLACE,

  SPR_LAB_BOSS_DIR_LEFT,
  SPR_LAB_BOSS_DIR_RIGHT,
  SPR_LAB_BOSS_DIR_UP,
  SPR_LAB_BOSS_DIR_DOWN,
  SPR_LAB_BOSS_STAY_AT_PLACE,

  SPR_TOUGH_BOSS_DIR_LEFT,
  SPR_TOUGH_BOSS_DIR_RIGHT,
  SPR_TOUGH_BOSS_DIR_UP,
  SPR_TOUGH_BOSS_DIR_DOWN,
  SPR_TOUGH_BOSS_STAY_AT_PLACE,

  SPR_DEMON_DIR_LEFT,
  SPR_DEMON_DIR_RIGHT,

  SPR_ROBOBOSS_DIR_LEFT,
  SPR_ROBOBOSS_DIR_RIGHT,

  SPR_UFO01_DIR_LEFT,
  SPR_UFO01_DIR_RIGHT,
  SPR_UFO01_DIR_UP,
  SPR_UFO01_DIR_DOWN,

  SPR_UFO02_DIR_LEFT,
  SPR_UFO02_DIR_RIGHT,
  SPR_UFO02_DIR_UP,
  SPR_UFO02_DIR_DOWN,

  SPR_ROBOTANK_DIR_LEFT,
  SPR_ROBOTANK_DIR_RIGHT,
  SPR_ROBOTANK_DIR_UP,
  SPR_ROBOTANK_DIR_DOWN,

  SPR_ZOMBIE_DIR_LEFT,
  SPR_ZOMBIE_DIR_RIGHT,
  SPR_ZOMBIE_DIR_UP,
  SPR_ZOMBIE_DIR_DOWN,

  SPR_ZOMBIE02_DIR_LEFT,
  SPR_ZOMBIE02_DIR_RIGHT,
  SPR_ZOMBIE02_DIR_UP,
  SPR_ZOMBIE02_DIR_DOWN,

  SPR_BOOM_UFO_STAY_AT_PLACE,
  SPR_BOOM_VIVI_STAY_AT_PLACE

};


enum {
      EL_ID_HERO = 0,
      EL_ID_HERO_ROCKET,
      EL_ID_STAR,
      EL_ID_BEAM,
      EL_ID_ENEMY_BALL_WEAPON,
      EL_ID_KNIFE,
      EL_ID_WRENCH,
      EL_ID_ENEMY_01,
      EL_ID_ENEMY_02,
      EL_ID_ENEMY_VIVI01,
      EL_ID_ENEMY_VIVI02,
      EL_ID_NUKE,
      EL_ID_LAB_BOSS,
      EL_ID_TOUGH_BOSS,
      EL_ID_TOUGH_ROCKET,
      EL_ID_DEMON,
      EL_ID_UFO01,
      EL_ID_UFO02,
      EL_ID_ROBOTANK,
      EL_ID_ROBOBOSS,
      EL_ID_ZOMBIE,
      EL_ID_ZOMBIE02,
      EL_ID_KATAFALK
     };


enum
{
  DIRECTION_UP = 0,
  DIRECTION_DOWN,
  DIRECTION_LEFT,
  DIRECTION_RIGHT,
  STATE_STAY_AT_PLACE
};


enum
{
  SND_BOSS_COMING = 0,
  SND_COLLECT_ANIMAL,
  SND_COLLECT_NUKE,
  SND_SHOT_CARROT,
  SND_WRENCH_SHOT,
  SND_POWERUP,
  SND_GAMEOVER,
  SND_VIVI_COLLISION,
  SND_ENEMY01_COLLISION,
  SND_UFO_COLLISION,
  SND_ROBOTANK_COLLISION,
  SND_ZOMBIE_COLLISION,
  SND_Y_COLLISION,

  SND_LABBOSS_COLLISION,
  SND_TBOSS_COLLISION,
  SND_DEMON_COLLISION,
  SND_ROBOBOSS_COLLISION,

  SND_KNIFE_SHOOT,
  SND_TBOSS_SHOOT,
  SND_DEMON_SHOOT,
  SND_ENEMY01_SHOOT
};


class CSprite: public QObject
{
public:

  QList <QImage> images;

  bool empty;

  int cycles_per_frame; //game loop cycles per frame
  int cycle_counter;

  int current_frame;

inline int get_current_frame_number()
{
  cycle_counter++;

  if (cycle_counter > 500)
     cycle_counter = 0;

  if (cycle_counter % cycles_per_frame == 0)
     current_frame++;

  if (current_frame == images.count())
     current_frame = 0;

  return current_frame;
}

  CSprite();
  void add_image (const QString &fname);
  CSprite* create_copy();
};


class CGameElement: public QObject
{
public:

  int id; //for example, 1 = hero
  int val; //for many purposes

  int snd_collision;
  int snd_shot;
  int fx;

  int initial_hp;
  int shoot_factor;


  CGameElement *weapon_shot;

  CGameElement();
  ~CGameElement();

  QRect element_area;

  int state; //was: int direction
  int speed;
  int hp; //hit points

  QString s_hp;

  bool can_shoot;

  CSprite* sprites [7];

  virtual CGameElement* create_self() = 0;
};


class CHero: public CGameElement
{
public:

  CHero();

  CGameElement* create_self();
};


class CStar: public CGameElement
{
public:

  CStar();
  CGameElement* create_self();
};


class CAnimal01: public CGameElement
{
public:

  CAnimal01();
  CGameElement* create_self();
};


class CAnimal02: public CGameElement
{
public:

  CAnimal02();
  CGameElement* create_self();
};


class CAnimal03: public CGameElement
{
public:

  CAnimal03();
  CGameElement* create_self();
};


class CNuke: public CGameElement
{
public:

  CNuke();
  CGameElement* create_self();
};


class CTrueLaserBeam: public CGameElement
{
public:

  CTrueLaserBeam();
  CGameElement* create_self();
};


class CUFO01: public CGameElement
{
public:

  CUFO01();
  CGameElement* create_self();
};


class CUFO02: public CGameElement
{
public:

  CUFO02();
  CGameElement* create_self();
};


class CEnemyNo1: public CGameElement
{
public:

  CEnemyNo1();
  CGameElement* create_self();
};


class CEnemyNo2: public CGameElement
{
public:

  CEnemyNo2();
  CGameElement* create_self();
};


class CRobotank: public CGameElement
{
public:

  CRobotank();
  CGameElement* create_self();
};


class CVivisector01: public CGameElement
{
public:

  CVivisector01();
  CGameElement* create_self();
};


class CVivisector02: public CGameElement
{
public:

  CVivisector02();
  CGameElement* create_self();
};


class CZombie: public CGameElement
{
public:

  CZombie();
  CGameElement* create_self();
};


class CZombie02: public CGameElement
{
public:

  CZombie02();
  CGameElement* create_self();
};



class CToughBoss: public CGameElement
{
public:

  CToughBoss();
  CGameElement* create_self();
};


class CRoboBoss: public CGameElement
{
public:

  CRoboBoss();
  CGameElement* create_self();
};


class CDemonBoss: public CGameElement
{
public:

  CDemonBoss();
  CGameElement* create_self();
};


class CLabBoss: public CGameElement
{
public:

  CLabBoss();
  CGameElement* create_self();
};


class CEnemyBallWeapon: public CGameElement
{
public:

  CEnemyBallWeapon();
  CGameElement* create_self();
};


class CWeaponKnife: public CGameElement
{
public:

  CWeaponKnife();
  CGameElement* create_self();
};


class CWeaponWrench: public CGameElement
{
public:

  CWeaponWrench();
  CGameElement* create_self();
};


class CCarrot: public CGameElement
{
public:

  CCarrot();
  CGameElement* create_self();
};


class CRocket: public CGameElement
{
public:

  CRocket();
  CGameElement* create_self();
};


class CKatafalk: public CGameElement
{
public:

  CKatafalk();
  CGameElement* create_self();
};


class CHeroRocket: public CGameElement
{
public:

  CHeroRocket();
  CGameElement* create_self();
};


class CFx: public CGameElement
{
public:

  CFx();
  CGameElement* create_self();
};


#endif // GAME_ELEMENTS_H
