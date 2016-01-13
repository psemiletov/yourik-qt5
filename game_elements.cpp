/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/


#include <QDebug>

#include "game_elements.h"


CSprite* sprite_pool[MAX_SPRITES];
CSampleList* sample_pool[MAX_SOUNDS];


CSprite::CSprite()
{
  current_frame = 0;
  cycle_counter = 0;
  cycles_per_frame = 20;
  empty = true;
}


void CSprite::add_image (const QString &fname)
{
  QImage im (fname);

  if (im.isNull())
     {
      qDebug() << fname << " is null";
      return;
     }

  images.append (im);
  empty = false;
}


CHero::CHero(): CGameElement()
{
  hp = 3;

  element_area.setX (0);
  element_area.setY (0);
  element_area.setHeight (64);
  element_area.setWidth (64);

  element_area.moveTo (1, RES_Y - 64);

  snd_collision = SND_Y_COLLISION;

  state = DIRECTION_RIGHT;

  speed = 5; //pixels per iteration

  sprites[DIRECTION_UP] = sprite_pool[SPR_HERO_DIR_UP]->create_copy();
  sprites[DIRECTION_DOWN] = sprite_pool[SPR_HERO_DIR_DOWN]->create_copy();
  sprites[DIRECTION_LEFT] = sprite_pool[SPR_HERO_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_HERO_DIR_RIGHT]->create_copy();

  id = EL_ID_HERO;
}


CStar::CStar()
{
  id = EL_ID_STAR;

  snd_collision = SND_POWERUP;

  state = STATE_STAY_AT_PLACE;
  sprites[STATE_STAY_AT_PLACE] = sprite_pool[SPR_STAR_STAY_AT_PLACE]->create_copy();

  element_area.setHeight (64);
  element_area.setWidth (64);
}


//dude, it's true. And it's damn hurts...
CTrueLaserBeam::CTrueLaserBeam()
{
  id = EL_ID_BEAM;
  speed = 15;

  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_BEAM_DIR_RIGHT]->create_copy();
  sprites[DIRECTION_LEFT] = sprite_pool[SPR_BEAM_DIR_LEFT]->create_copy();
  sprites[DIRECTION_UP] = sprite_pool[SPR_BEAM_DIR_UP]->create_copy();
  sprites[DIRECTION_DOWN] = sprite_pool[SPR_BEAM_DIR_DOWN]->create_copy();

  element_area.setWidth (64);
  element_area.setHeight (16);
}


CUFO01::CUFO01()
{
  id = EL_ID_UFO01;

  snd_collision = SND_UFO_COLLISION;
  snd_shot = SND_ENEMY01_SHOOT;
  fx = SPR_BOOM_UFO_STAY_AT_PLACE;

  speed = 2;
  can_shoot = true;
  shoot_factor = 500;
  val = 0;

  sprites[DIRECTION_LEFT] = sprite_pool[SPR_UFO01_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_UFO01_DIR_RIGHT]->create_copy();

  element_area.setWidth (64);
  element_area.setHeight (48);

  state = DIRECTION_RIGHT;
}


CGameElement* CUFO01::create_self()
{
  return new CUFO01;
}


CUFO02::CUFO02()
{
  id = EL_ID_UFO02;

  snd_collision = SND_UFO_COLLISION;
  snd_shot = SND_ENEMY01_SHOOT;
  fx = SPR_BOOM_UFO_STAY_AT_PLACE;

  speed = 3;
  can_shoot = true;
  shoot_factor = 500;
  val = 0;

  sprites[DIRECTION_LEFT] = sprite_pool[SPR_UFO02_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_UFO02_DIR_RIGHT]->create_copy();

  element_area.setWidth (64);
  element_area.setHeight (48);

  state = DIRECTION_RIGHT;
}


CGameElement* CUFO02::create_self()
{
  return new CUFO02;
}


CEnemyNo1::CEnemyNo1()
{
  id = EL_ID_ENEMY_01;

  can_shoot = true;

  speed = 2;
  val = 0;
  shoot_factor = 350;

  snd_collision = SND_ENEMY01_COLLISION;
  snd_shot = SND_ENEMY01_SHOOT;

  sprites[DIRECTION_UP] = sprite_pool[SPR_ENEMY01_DIR_UP]->create_copy();
  sprites[DIRECTION_DOWN] = sprite_pool[SPR_ENEMY01_DIR_DOWN]->create_copy();
  sprites[DIRECTION_LEFT] = sprite_pool[SPR_ENEMY01_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_ENEMY01_DIR_RIGHT]->create_copy();

  element_area.setWidth (64);
  element_area.setHeight (64);
}


CEnemyBallWeapon::CEnemyBallWeapon()
{
  id = EL_ID_ENEMY_BALL_WEAPON;

  speed = 4;

  sprites[DIRECTION_UP] = sprite_pool[SPR_ENEMY_BALL_WEAPON_DIR_UP]->create_copy();
  sprites[DIRECTION_DOWN] = sprite_pool[SPR_ENEMY_BALL_WEAPON_DIR_DOWN]->create_copy();
  sprites[DIRECTION_LEFT] = sprite_pool[SPR_ENEMY_BALL_WEAPON_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_ENEMY_BALL_WEAPON_DIR_RIGHT]->create_copy();

  element_area.setWidth (16);
  element_area.setHeight (16);
}


CToughBoss::CToughBoss()
{
  id = EL_ID_TOUGH_BOSS;

  initial_hp = 5;

  hp = initial_hp;

  speed = 3;

  can_shoot = true;

  shoot_factor = 100;

  snd_collision = SND_TBOSS_COLLISION;
  snd_shot = SND_TBOSS_SHOOT;

  sprites[DIRECTION_LEFT] = sprite_pool[SPR_TOUGH_BOSS_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_TOUGH_BOSS_DIR_RIGHT]->create_copy();

  element_area.setWidth (128);
  element_area.setHeight (100);

  weapon_shot = new CTrueLaserBeam;

  state = DIRECTION_RIGHT;

  element_area.moveTo (0, 0);
}


CGameElement* CEnemyNo1::create_self()
{
  return new CEnemyNo1;
}


CGameElement* CToughBoss::create_self()
{
  return new CToughBoss;
}


CGameElement* CLabBoss::create_self()
{
  return new CLabBoss;
}


CLabBoss::CLabBoss()
{
  id = EL_ID_LAB_BOSS;

  initial_hp = 5;

  hp = initial_hp;

  speed = 3;
  snd_collision = SND_LABBOSS_COLLISION;
  snd_shot = SND_KNIFE_SHOOT;
  shoot_factor = 100;
  can_shoot = true;

  sprites[DIRECTION_LEFT] = sprite_pool[SPR_LAB_BOSS_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_LAB_BOSS_DIR_RIGHT]->create_copy();

  element_area.setWidth (128);
  element_area.setHeight (148);

  weapon_shot = new CWeaponKnife;

  state = DIRECTION_RIGHT;

  element_area.moveTo (0, 0);
}


CGameElement* CStar::create_self()
{
  return new CStar;
}


CEnemyNo2::CEnemyNo2()
{
  id = EL_ID_ENEMY_02;
  speed = 3;

  can_shoot = true;
  shoot_factor = 350;
  val = 0;

  snd_collision = SND_ENEMY01_COLLISION;
  snd_shot = SND_ENEMY01_SHOOT;

  sprites[DIRECTION_LEFT] = sprite_pool[SPR_ENEMY02_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_ENEMY02_DIR_RIGHT]->create_copy();
  sprites[DIRECTION_UP] = sprite_pool[SPR_ENEMY02_DIR_UP]->create_copy();
  sprites[DIRECTION_DOWN] = sprite_pool[SPR_ENEMY02_DIR_DOWN]->create_copy();

  element_area.setWidth (64);
  element_area.setHeight (64);
}


CGameElement* CEnemyNo2::create_self()
{
  return new CEnemyNo2;
}


CGameElement* CHero::create_self()
{
  return new CHero;
}


CGameElement* CTrueLaserBeam::create_self()
{
  return new CTrueLaserBeam;
}


CGameElement* CEnemyBallWeapon::create_self()
{
  return new CEnemyBallWeapon;
}


CCarrot::CCarrot()
{
  speed = 10;

  sprites[DIRECTION_LEFT] = sprite_pool[SPR_CARROT_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_CARROT_DIR_RIGHT]->create_copy();
  sprites[DIRECTION_UP] = sprite_pool[SPR_CARROT_DIR_UP]->create_copy();
  sprites[DIRECTION_DOWN] = sprite_pool[SPR_CARROT_DIR_DOWN]->create_copy();

  element_area.setWidth (64);
  element_area.setHeight (16);
}


CGameElement* CCarrot::create_self()
{
  return new CCarrot;
}


CAnimal01::CAnimal01()
{
  snd_collision = SND_COLLECT_ANIMAL;

  state = STATE_STAY_AT_PLACE;
  sprites[STATE_STAY_AT_PLACE] = sprite_pool[SPR_ANIMAL01]->create_copy();
  element_area.setWidth (48);
  element_area.setHeight (64);
}


CGameElement* CAnimal01::create_self()
{
  return new CAnimal01;
}


CAnimal02::CAnimal02()
{
  snd_collision = SND_COLLECT_ANIMAL;

  state = STATE_STAY_AT_PLACE;
  sprites[STATE_STAY_AT_PLACE] = sprite_pool[SPR_ANIMAL02]->create_copy();

  element_area.setWidth (48);
  element_area.setHeight (54);
}


CAnimal03::CAnimal03()
{
  snd_collision = SND_COLLECT_ANIMAL;

  state = STATE_STAY_AT_PLACE;
  sprites[STATE_STAY_AT_PLACE] = sprite_pool[SPR_ANIMAL03]->create_copy();

  element_area.setWidth (48);
  element_area.setHeight (64);
}


CGameElement* CAnimal02::create_self()
{
  return new CAnimal02;
}


CGameElement* CAnimal03::create_self()
{
  return new CAnimal03;
}


CVivisector01::CVivisector01()
{
  id = EL_ID_ENEMY_VIVI01;

  speed = 2;

  val = 0;

  snd_collision = SND_VIVI_COLLISION;
  //fx = SPR_BOOM_VIVI_STAY_AT_PLACE;

  can_shoot = false;

  sprites[DIRECTION_LEFT] = sprite_pool[SPR_VIVI01_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_VIVI01_DIR_RIGHT]->create_copy();
  sprites[DIRECTION_UP] = sprite_pool[SPR_VIVI01_DIR_UP]->create_copy();
  sprites[DIRECTION_DOWN] = sprite_pool[SPR_VIVI01_DIR_DOWN]->create_copy();

  element_area.setWidth (48);
  element_area.setHeight (64);
}


CGameElement* CVivisector01::create_self()
{
  return new CVivisector01;
}


CVivisector02::CVivisector02()
{
  id = EL_ID_ENEMY_VIVI02;

  speed = 1;

  can_shoot = false;

  snd_collision = SND_VIVI_COLLISION;
//  fx = SPR_BOOM_VIVI_STAY_AT_PLACE;

  val = 0;

  sprites[DIRECTION_LEFT] = sprite_pool[SPR_VIVI02_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_VIVI02_DIR_RIGHT]->create_copy();
  sprites[DIRECTION_UP] = sprite_pool[SPR_VIVI02_DIR_UP]->create_copy();
  sprites[DIRECTION_DOWN] = sprite_pool[SPR_VIVI02_DIR_DOWN]->create_copy();

  element_area.setWidth (48);
  element_area.setHeight (64);
}


CGameElement* CVivisector02::create_self()
{
  return new CVivisector02;
}


CNuke::CNuke()
{
  state = STATE_STAY_AT_PLACE;
  sprites[STATE_STAY_AT_PLACE] = sprite_pool[SPR_NUKE_STAY_AT_PLACE]->create_copy();

  element_area.setWidth (64);
  element_area.setHeight (64);

  id = EL_ID_NUKE;

  snd_collision = SND_COLLECT_NUKE;
}


CGameElement* CNuke::create_self()
{
  return new CNuke;
}


CWeaponKnife::CWeaponKnife()
{
  id = EL_ID_KNIFE;

  speed = 3;

  sprites[DIRECTION_DOWN] = sprite_pool[SPR_KNIFE_DIR_DOWN]->create_copy();

  element_area.setWidth (64);
  element_area.setHeight (64);
}


CGameElement* CWeaponKnife::create_self()
{
  return new CWeaponKnife;
}


CWeaponWrench::CWeaponWrench()
{
  id = EL_ID_WRENCH;

  speed = 3;

  sprites[DIRECTION_DOWN] = sprite_pool[SPR_WRENCH_DIR_DOWN]->create_copy();

  element_area.setWidth (64);
  element_area.setHeight (64);
}


CGameElement* CWeaponWrench::create_self()
{
  return new CWeaponWrench;
}


CGameElement::CGameElement()
{
  fx = -1;
  weapon_shot = 0;
  can_shoot = false;
  shoot_factor = 0;

  initial_hp = 7;;

  snd_collision = 0;
  snd_shot = 0;

  s_hp = "0";

  sprites[0] = 0;
  sprites[1] = 0;
  sprites[2] = 0;
  sprites[3] = 0;
  sprites[4] = 0;
}


CGameElement::~CGameElement()
{
  if (weapon_shot)
     delete weapon_shot;

  for (int i = 0; i < 5; i++)
      if (sprites[i])
          delete sprites[i];
}


CRocket::CRocket()
{
  id = EL_ID_TOUGH_ROCKET;

  state = STATE_STAY_AT_PLACE;

  speed = 0;

  sprites[STATE_STAY_AT_PLACE] = sprite_pool[SPR_ROCKET_STAY_AT_PLACE]->create_copy();

  element_area.setWidth (164);
  element_area.setHeight (94);
  element_area.moveTo (10, 10);
}


CGameElement* CRocket::create_self()
{
  return new CRocket;
}


CHeroRocket::CHeroRocket()
{
  hp = 3;

  element_area.setX (0);
  element_area.setY (0);
  element_area.setHeight (100);
  element_area.setWidth (48);

  element_area.moveTo (1, RES_Y - 100);

  state = DIRECTION_RIGHT;
  snd_collision = SND_Y_COLLISION;

  speed = 5;

  sprites[DIRECTION_LEFT] = sprite_pool[SPR_HERO_ROCKET_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_HERO_ROCKET_DIR_RIGHT]->create_copy();
  sprites[DIRECTION_UP] = sprite_pool[SPR_HERO_ROCKET_DIR_UP]->create_copy();
  sprites[DIRECTION_DOWN] = sprite_pool[SPR_HERO_ROCKET_DIR_DOWN]->create_copy();

  id = EL_ID_HERO_ROCKET;
}


CGameElement* CHeroRocket::create_self()
{
  return new CHeroRocket;
}


CDemonBoss::CDemonBoss()
{
  id = EL_ID_DEMON;

  initial_hp = 7;
  hp = initial_hp;
  speed = 3;
  shoot_factor = 80;
  can_shoot = true;
  snd_collision = SND_DEMON_COLLISION;
  snd_shot = SND_DEMON_SHOOT;

  sprites[DIRECTION_LEFT] = sprite_pool[SPR_DEMON_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_DEMON_DIR_RIGHT]->create_copy();

  element_area.setWidth (280);
  element_area.setHeight (230);

  weapon_shot = new CTrueLaserBeam;

  state = DIRECTION_RIGHT;

  element_area.moveTo (0, 0);
}


CGameElement* CDemonBoss::create_self()
{
  return new CDemonBoss;
}


CRobotank::CRobotank()
{
  id = EL_ID_ROBOTANK;
  speed = 1;

  can_shoot = true;
  shoot_factor = 350;
  val = 0;
  snd_collision = SND_ROBOTANK_COLLISION;
  snd_shot = SND_ENEMY01_SHOOT;

  sprites[DIRECTION_LEFT] = sprite_pool[SPR_ROBOTANK_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_ROBOTANK_DIR_RIGHT]->create_copy();
  sprites[DIRECTION_UP] = sprite_pool[SPR_ROBOTANK_DIR_UP]->create_copy();
  sprites[DIRECTION_DOWN] = sprite_pool[SPR_ROBOTANK_DIR_DOWN]->create_copy();

  element_area.setWidth (64);
  element_area.setHeight (64);
}


CGameElement* CRobotank::create_self()
{
  return new CRobotank;
}


CRoboBoss::CRoboBoss()
{
  id = EL_ID_ROBOBOSS;

  initial_hp = 9;

  hp = initial_hp;
  speed = 3;
  can_shoot = true;
  shoot_factor = 90;

  snd_collision = SND_ROBOBOSS_COLLISION;
  snd_shot = SND_WRENCH_SHOT;

  sprites[DIRECTION_LEFT] = sprite_pool[SPR_ROBOBOSS_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_ROBOBOSS_DIR_RIGHT]->create_copy();

  element_area.setWidth (148);
  element_area.setHeight (200);

  weapon_shot = new CWeaponWrench;

  state = DIRECTION_RIGHT;

  element_area.moveTo (0, 0);
}


CGameElement* CRoboBoss::create_self()
{
  return new CRoboBoss;
}


CZombie::CZombie()
{
  id = EL_ID_ZOMBIE;

  speed = 1;
  val = 0;
  can_shoot = false;
  snd_collision = SND_ZOMBIE_COLLISION;

  sprites[DIRECTION_LEFT] = sprite_pool[SPR_ZOMBIE_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_ZOMBIE_DIR_RIGHT]->create_copy();
  sprites[DIRECTION_UP] = sprite_pool[SPR_ZOMBIE_DIR_UP]->create_copy();
  sprites[DIRECTION_DOWN] = sprite_pool[SPR_ZOMBIE_DIR_DOWN]->create_copy();

  element_area.setWidth (48);
  element_area.setHeight (64);
}


CGameElement* CZombie::create_self()
{
  return new CZombie;
}


CZombie02::CZombie02()
{
  id = EL_ID_ZOMBIE02;

  speed = 1;
  val = 0;
  snd_collision = SND_ZOMBIE_COLLISION;
  can_shoot = false;

  sprites[DIRECTION_LEFT] = sprite_pool[SPR_ZOMBIE02_DIR_LEFT]->create_copy();
  sprites[DIRECTION_RIGHT] = sprite_pool[SPR_ZOMBIE02_DIR_RIGHT]->create_copy();
  sprites[DIRECTION_UP] = sprite_pool[SPR_ZOMBIE02_DIR_UP]->create_copy();
  sprites[DIRECTION_DOWN] = sprite_pool[SPR_ZOMBIE02_DIR_DOWN]->create_copy();

  element_area.setWidth (48);
  element_area.setHeight (64);
}


CGameElement* CZombie02::create_self()
{
  return new CZombie02;
}


CKatafalk::CKatafalk()
{
  id = EL_ID_KATAFALK;

  state = STATE_STAY_AT_PLACE;

  speed = 0;

  sprites[STATE_STAY_AT_PLACE] = sprite_pool[SPR_KATAFALK_STAY_AT_PLACE]->create_copy();

  element_area.setWidth (274);
  element_area.setHeight (446);
  element_area.moveTo (10, -200);
}


CGameElement* CKatafalk::create_self()
{
  return new CKatafalk;
}


CFx::CFx()
{
  hp = 50;
  state = STATE_STAY_AT_PLACE;
}


CGameElement* CFx::create_self()
{
  return new CFx;
}


CSprite* CSprite::create_copy()
{
  CSprite *s = new CSprite;
  s->current_frame = 0;
  s->cycles_per_frame = cycles_per_frame;

  foreach (QImage im, images)
          {
           s->images.append (im);
          }

  return s;
}
