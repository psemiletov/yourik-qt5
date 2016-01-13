/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/

#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

//#include <QtGui>
//#include <QtOpenGL>

#include <QObject>
#include <QList>
#include <QImage>
#include <QPoint>
#include <QString>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QGLWidget>
#include <QTimer>
#include <QMainWindow>
#include <QSettings>


#include "game_elements.h"
#include "finesound.h"


enum {
      ITERATION_GAME,
      ITERATION_TITLE
};


enum {
      ANIMATYPE_STAY,
      ANIMATYPE_MOVE_LR
};


enum {
      LVL_GOAL_COLLECT,
      LVL_GOAL_KILL, //not used
      LVL_GOAL_KILL_BOSS,
      LVL_GOAL_SPOTPLACE
};


class CTitle: public QObject
{
   Q_OBJECT

public:

  QImage image;
  QString text;
  QColor color;

  QPoint pos;

  int duration; //seconds

  bool is_image;

  CTitle (const QString &val, int seconds = 1, int x = 0, int y = 0, bool image_mode = false, QColor cl = QColor ("white"));
};


class CTitles: public QObject
{
  Q_OBJECT

public:

  QList <CTitle *> list;
  int current_title;

  CTitles();
   ~CTitles();

  bool load_from_file (const QString &fname);
};


class CAnimabackground: public QObject
{
 Q_OBJECT

public:

  CSprite sprite;
  QPoint position;

  int animatype;

  void load (const QString &fname);
};


class CGameLevel: public QObject
{
  Q_OBJECT

public:

  QString level_name;
  QString next_level;
  QString death_level;

  QList <CGameElement *> collectable_types;
  QList <CGameElement *> enemy_types;

  CGameElement *hero;

  CGameElement *boss;
  CGameElement *goal_place;

  QImage background;

  CMusic *music;
  CMusic *titles_music;

  CAnimabackground abackground;

  CTitles *titles;

  int level_goal;

  int collectable_goal;
  int enemies_to_kill_before_boss;
  int enemies_to_kill; //to finish the level when the goal is GOAL_KILL

  int places_checked;

  bool goal_gained;

  int enemies_per_wave;
  int stars_per_wave;
  int collectables_per_wave;

  CGameLevel();
  ~CGameLevel();
};


class CGameLevel_1: public CGameLevel
{
   Q_OBJECT

public:

  CGameLevel_1();
};


class CGameLevel_2: public CGameLevel
{
   Q_OBJECT

public:

  CGameLevel_2();
};


class CGameLevel_3: public CGameLevel
{
   Q_OBJECT

public:

  CGameLevel_3();
};


class CGameLevel_4: public CGameLevel
{
   Q_OBJECT

public:

  CGameLevel_4();
};


class CGameLevel_5: public CGameLevel
{
   Q_OBJECT

public:

  CGameLevel_5();
};


class CGameLevel_6: public CGameLevel
{
   Q_OBJECT

public:

  CGameLevel_6();
};


class CGameLevel_luna: public CGameLevel
{
   Q_OBJECT

public:

  CGameLevel_luna();
};


class CGameLevel_after_luna: public CGameLevel
{
   Q_OBJECT

public:

  CGameLevel_after_luna();
};


class CGameLevel_gameover: public CGameLevel
{
   Q_OBJECT

public:

  CGameLevel_gameover();
};


class CGameArea: public QGLWidget
{
   Q_OBJECT

public:

  int iteration_mode; //is game, or title

  QTimer timer_game;
  QTimer timer_title;

  CSoundEngine *sound_engine;

  CGameLevel *current_level;
  CTitles *current_titles;

  QList <CGameElement *> stars;
  QList <CGameElement *> enemies;
  QList <CGameElement *> collectables;
  QList <CGameElement *> enemy_ball_weapons;
  QList <CGameElement *> fx;

  CGameElement *hero;
  CGameElement *carrot;
  CGameElement *boss;
  CGameElement *goal_place;

  QImage background_image;
  QImage title_screen;
  QImage status_heart;

  bool level_goal_gained;
  bool create_carrot;

  int cycles_per_second;
  int current_cycle;

  int enemies_to_kill_before_boss;
  int collected_prize_goal;
  int enemies_to_kill; //not used
  int places_checked;

  QString s_hp;
  QString s_collected_prize_goal;
  QString s_killed_goal;

  CGameArea();
  ~CGameArea();

  void basic_move (CGameElement *e);
  void iter_mode_game();
  void iter_mode_title();

  CGameLevel *get_level (const QString &lname);

  void setup_level (bool death = false);

  void init_stars();
  void init_collectables();

  void create_boss();
  void create_enemies();
  void make_carrot();

  void iteration();
  void check_intersections();
  void enemies_shoot();

  void init_sprite_pool();
  void done_sprite_pool();

  void init_sample_pool();
  void done_sample_pool();

  void delete_objects();


protected:

  void	paintEvent (QPaintEvent *event);
  void keyPressEvent (QKeyEvent *event);

public slots:

  void iter_game();
  void iter_title();
};


class CMainWindow : public QMainWindow
{
    Q_OBJECT

public:

    QString dir_config;

    QSettings *settings;

    CGameArea *game_area;

    CMainWindow (QWidget *parent = 0);
    ~CMainWindow();

    void readSettings();
    void writeSettings();

protected:

  void closeEvent (QCloseEvent *event);
};

#endif // CMAINWINDOW_H
