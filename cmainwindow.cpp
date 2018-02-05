/***********************************************************
 *   this code by Peter Semiletov is Public Domain         *
 **********************************************************/

//20 nov 2010

#include <QDebug>
#include <QDir>
#include <QApplication>
#include <QTime>
#include <QCloseEvent>

#include "cmainwindow.h"
#include "utils.h"


#define INIT_LEVEL "level01"

#define MAX_HP 7


QString dir_snd;
QString dir_pix;
QString dir_titles;

extern CSprite* sprite_pool[MAX_SPRITES];
extern CSampleList* sample_pool[MAX_SOUNDS];

bool godmode;
bool devel_mode;

QString locale_override;
QString initial_level;

int area_width;
int area_height;

int dest_res_x;
int dest_res_y;


void CMainWindow::writeSettings()
{
  settings->setValue ("pos", pos());
  settings->setValue ("width", dest_res_x);
  settings->setValue ("height", dest_res_y);

  delete settings;
}


void CMainWindow::readSettings()
{
  QDir dr;
  dir_config = dr.homePath();

#ifdef Q_WS_WIN

  dir_config.append ("/yourik");

#else

  dir_config.append ("/.config/yourik");

#endif

  dr.setPath (dir_config);
  if (! dr.exists())
     dr.mkpath (dir_config);

  QString sfilename = dir_config + "/y.conf";
  settings = new QSettings (sfilename, QSettings::IniFormat);

  QPoint pos = settings->value ("pos", QPoint (1, 1)).toPoint();
  move (pos);

  dest_res_x = settings->value ("width", 800).toInt();
  dest_res_y = settings->value ("height", 600).toInt();


}


CMainWindow::CMainWindow (QWidget *parent): QMainWindow(parent)
{
  readSettings();

  resize (QSize (dest_res_x, dest_res_y));

//  resize (QSize (RES_X, RES_Y));

  godmode = false;
  devel_mode = false;
  initial_level = INIT_LEVEL;

  if (qApp->arguments().count() > 2)
     {
      qDebug() << "devel mode = true";
      devel_mode = true;
      locale_override = qApp->arguments().at (2);
      initial_level = qApp->arguments().at (1);
     }

  game_area = new CGameArea();
  setCentralWidget (game_area);
  game_area->setFocus (Qt::OtherFocusReason);
}


CMainWindow::~CMainWindow()
{
  writeSettings();
  delete game_area;
}


void CGameArea::iter_game()
{
  iteration();
}


void CMainWindow::closeEvent (QCloseEvent *event)
{
  event->accept();
}


CGameArea::CGameArea()
{
  qDebug() << "CGameArea() - start";

  area_width = SOURCE_RES_X;
  area_height = SOURCE_RES_Y;
  
  offscreen_buffer = new QImage (SOURCE_RES_X, SOURCE_RES_Y, QImage::Format_RGB32);
  onscreen_buffer = new QImage (dest_res_x, dest_res_y, QImage::Format_RGB32);
  

#ifdef Q_WS_WIN

  dir_snd = QCoreApplication::applicationDirPath() + "\\snd\\";
  dir_pix = QCoreApplication::applicationDirPath() + "\\pix\\";
  dir_titles = QCoreApplication::applicationDirPath() + "\\titles\\";

#else

  dir_pix = PIXDIR;
  dir_snd = SNDDIR;
  dir_titles = TITLESDIR;

#endif

//FIXME!!!!!!! for pre-release only

  if (devel_mode)
     {
      dir_pix = QCoreApplication::applicationDirPath() + "/pix/";
      dir_snd = QCoreApplication::applicationDirPath() + "/snd/";
      dir_titles = QCoreApplication::applicationDirPath() + "/titles/";

      qDebug() << "dir_titles = " << dir_titles;
      qDebug() << "dir_pix = " << dir_pix;
      qDebug() << "dir_snd = " << dir_snd;
     }

  qDebug() << "dir_titles = " << dir_titles;
  qDebug() << "dir_pix = " << dir_pix;
  qDebug() << "dir_snd = " << dir_pix;

  sound_engine = new CSoundEngine;

  init_sprite_pool();
  init_sample_pool();

  status_heart.load (dir_pix + "status-heart.png");

  current_level = 0;
  s_killed_goal = "0";

  level_goal_gained = false;
  create_carrot = false;

  current_cycle = 0;
  carrot = 0;
  boss = 0;
  hero = 0;
  current_titles = 0;

  setup_level();

/////////////

  iter_mode_title();

//  iter_mode_game();

//////////

  cycles_per_second = CYC_PER_SECOND;

  timer_game.setInterval (1000 / cycles_per_second);
  connect (&timer_game, SIGNAL(timeout()), this, SLOT (iter_game()));

  int title_cycles_per_second = 1;
  timer_title.setInterval (1000 / title_cycles_per_second);
  connect (&timer_title, SIGNAL(timeout()), this, SLOT (iter_title()));

  qDebug() << "CGameArea() - end";
}


void CGameArea::iteration()
{
//qDebug() << "CGameArea::iteration() - start";

  current_cycle++;

  if (current_cycle > cycles_per_second)
     current_cycle = 0;

  if (current_level->level_goal == LVL_GOAL_COLLECT)
  if (collected_prize_goal <= 0)
      level_goal_gained = true;

  if (current_level->level_goal == LVL_GOAL_SPOTPLACE)
     if (places_checked <= 0)
        level_goal_gained = true;

//actually, LVL_GOAL_KILL is not used
  if (current_level->level_goal == LVL_GOAL_KILL)
     if (enemies_to_kill <= 0)
        level_goal_gained = true;


  if (hero->hp <= 0)
     {
      if (current_level->level_name != "level04")
         sound_engine->play_sample (sample_pool[SND_GAMEOVER]->get());

      setup_level (true);
      iter_mode_title();
      return;
     }

  if (level_goal_gained)
     {
      setup_level();
      iter_mode_title();
      return;
     }

  if (enemies.count() <= 0)
  if (! boss)
      create_enemies();

  enemies_shoot();

  if (enemies_to_kill_before_boss <= 0)
     {
      create_boss();
      enemies_to_kill_before_boss = current_level->enemies_to_kill_before_boss;
     }

//hero movement

  basic_move (hero);

  if (create_carrot)
     make_carrot();

//enemies movement

  if (hero->id != EL_ID_HERO_ROCKET)
  for (int i = 0; i < enemies.count(); i++)
      {
       CGameElement *e = enemies[i];

       int x_state;
       int y_state;

       if (hero->element_area.x() <= e->element_area.x())
          x_state = DIRECTION_LEFT;

       if (hero->element_area.x() >= e->element_area.x())
          x_state = DIRECTION_RIGHT;

       if (hero->element_area.y() >= e->element_area.y())
          y_state = DIRECTION_DOWN;

       if (hero->element_area.y() <= e->element_area.y())
          y_state = DIRECTION_UP;

       //x or y state?

       //ближе по вертикали или по горизонтали?

       int distance_x = 0;
       int distance_y = 0;

       if (hero->element_area.x() > e->element_area.x())
          distance_x = hero->element_area.x() - e->element_area.x();
       else
          distance_x = e->element_area.x() - hero->element_area.x();

       if (hero->element_area.y() > e->element_area.y())
          distance_y = hero->element_area.y() - e->element_area.y();
       else
          distance_y = e->element_area.y() - hero->element_area.y();

       if (distance_x < distance_y)
          e->state = y_state;
       else
       if (distance_x > distance_y)
           e->state = x_state;
       else
       if (distance_x == distance_y)
          {
           if (qrand() % 2 == 1)
              e->state = x_state;
           else
              e->state = y_state;
          }

       basic_move (e);
      }
  else
  for (int i = 0; i < enemies.count(); i++)
      {
       CGameElement *e = enemies[i];

       int opposite_state = DIRECTION_LEFT;

       if (opposite_state == DIRECTION_LEFT)
           opposite_state = DIRECTION_RIGHT;
        else
           opposite_state = DIRECTION_LEFT;

       if (e->element_area.x() + e->element_area.width() + 8 >= area_width)
             e->state = DIRECTION_LEFT;

       if (e->element_area.x() <= 0)
             e->state = DIRECTION_RIGHT;

       basic_move (e);
      }

//boss movement

  if (boss)
     {
      int new_x = boss->element_area.x();
      int new_y = boss->element_area.y();

      switch (boss->state)
             {
              case DIRECTION_LEFT:
                                new_x -= boss->speed;
                                break;

              case DIRECTION_RIGHT:
                                new_x += boss->speed;
                                break;
             };


      if (new_x < 0)
         boss->state = DIRECTION_RIGHT;

      if (new_x + boss->element_area.width() > area_width)
         boss->state = DIRECTION_LEFT;

      if (boss)
         boss->element_area.moveTo (new_x, new_y);
    }

//super carrot movement

  if (carrot)
     {
      int new_x = carrot->element_area.x();
      int new_y = carrot->element_area.y();

      switch (carrot->state)
             {
              case DIRECTION_UP:
                                new_y -= carrot->speed;
                                break;

              case DIRECTION_DOWN:
                                new_y += carrot->speed;
                                break;

              case DIRECTION_LEFT:
                                new_x -= carrot->speed;
                                break;

              case DIRECTION_RIGHT:
                                new_x += carrot->speed;
                                break;
             };

      if (new_y < 0)
         {
          delete carrot;
          carrot = 0;
         }
      else
      if (new_x < 0)
         {
          delete carrot;
          carrot = 0;
         }
      else
      if (new_x > area_width)
         {
          delete carrot;
          carrot = 0;
         }
      else
      if (new_y > area_height)
         {
          delete carrot;
          carrot = 0;
         }

      if (carrot)
         carrot->element_area.moveTo (new_x, new_y);
     }


//enemy bullets movement

  foreach (CGameElement *ebw, enemy_ball_weapons)
          {
           int new_x = ebw->element_area.x();
           int new_y = ebw->element_area.y();

           switch (ebw->state)
                  {
                   case DIRECTION_UP:
                                     new_y -= ebw->speed;
                                     break;

                   case DIRECTION_DOWN:
                                      new_y += ebw->speed;
                                      break;

                   case DIRECTION_LEFT:
                                      new_x -= ebw->speed;
                                      break;

                   case DIRECTION_RIGHT:
                                       new_x += ebw->speed;
                                      break;
                   };


          if (new_y < 0)
             {
              enemy_ball_weapons.removeOne (ebw);
              delete ebw;
              ebw = 0;
             }
          else
          if (new_x < 0)
             {
              enemy_ball_weapons.removeOne (ebw);
              delete ebw;
              ebw = 0;
             }
          else
          if (new_x > area_width)
             {
              enemy_ball_weapons.removeOne (ebw);
              delete ebw;
              ebw = 0;
             }
          else
          if (new_y > area_height)
             {
              enemy_ball_weapons.removeOne (ebw);
              delete ebw;
              ebw = 0;
             }

          if (ebw)
             ebw->element_area.moveTo (new_x, new_y);
        }


//check all
  check_intersections();

  if (current_level)
     {
      //if ((! current_level->abackground.image.isNull()))
        // {
          /*if (current_level->abackground.animatype == ANIMATYPE_ROTATE) //too hard for cpu
             {
              current_level->abackground.angle++;
              if (current_level->abackground.angle > 360)
                 current_level->abackground.angle = 0;

              QTransform transf;

              transf.rotate (current_level->abackground.angle);

              current_level->abackground.render_image = current_level->abackground.image.transformed (transf);
             }
*/

         if (! current_level->abackground.sprite.empty)
         if (current_level->abackground.animatype == ANIMATYPE_MOVE_LR)
            {
             //qDebug() << "ANIMATYPE_MOVE_LR";
             int new_x = current_level->abackground.position.x() - 1;
             if (new_x + current_level->abackground.sprite.images[0].width() < 0)
                new_x = area_width;

             current_level->abackground.position.setX (new_x);
            }


     if (stars.count() == 0)
       init_stars(); //why here?

     init_collectables(); //why here?
    }


  foreach (CGameElement *e, fx)
          {
           //the Right Thing:
           //if (e->sprites[e->state]->current_frame == e->sprites[e->state]->images.count() - 1)

           //the Wroooong Way:
           if (--e->hp < 0) //more fancy ;)
              {
               e->sprites[e->state]->current_frame = 0;
               fx.removeOne (e);
               delete e;
              }
          }

  update();

 //qDebug() << "CGameArea::iteration() - end";
}


CGameArea::~CGameArea()
{
  timer_game.stop();
  timer_title.stop();

  if (carrot)
     delete carrot;

  delete_objects();

  done_sprite_pool();
  done_sample_pool();

  delete sound_engine;
  
  delete offscreen_buffer;
  delete onscreen_buffer; 
}


void CGameArea::paintEvent (QPaintEvent * event)
{
//qDebug() << "paintEvent ";

  if (iteration_mode == ITERATION_GAME)
     {
      QPainter painter (offscreen_buffer);

      painter.drawImage (0, 0, background_image);

      if (current_level)
          if (! current_level->abackground.sprite.empty)
              painter.drawImage (current_level->abackground.position.x(), current_level->abackground.position.y(),
                                 current_level->abackground.sprite.images[current_level->abackground.sprite.get_current_frame_number()]);


      for (int i = 0; i < fx.count(); i++)
          {
           CGameElement *el = fx[i];
           CSprite *sprite = el->sprites[el->state];

           painter.drawImage (el->element_area.x(), el->element_area.y(),
                              sprite->images[sprite->get_current_frame_number()]);
          }


      for (int i = 0; i < collectables.count(); i++)
          {
           CGameElement *e = collectables[i];

           CSprite *e_sprite = e->sprites[e->state];
           painter.drawImage (e->element_area.x(), e->element_area.y(),
                              e_sprite->images[e_sprite->get_current_frame_number()]);
          }


      for (int i = 0; i < stars.count(); i++)
          {
           CGameElement *star = stars[i];
           CSprite *star_sprite = star->sprites[star->state];
           if (! star_sprite)
              qDebug() << "! star_sprit";

           painter.drawImage (star->element_area.x(), star->element_area.y(),
                              star_sprite->images[star_sprite->get_current_frame_number()]);
          }


      for (int i = 0; i < enemies.count(); i++)
          {
           CGameElement *e = enemies[i];

           CSprite *en1_sprite = e->sprites[e->state];
           if (! en1_sprite)
              qDebug() << "! en1_sprite";

           painter.drawImage (e->element_area.x(), e->element_area.y(),
                              en1_sprite->images[en1_sprite->get_current_frame_number()]);
          }


      if (boss)
         {
          CSprite *s = boss->sprites[boss->state];
          if (! s)
             qDebug() << "! s";

          painter.drawImage (boss->element_area.x(), boss->element_area.y(),
                             s->images[s->get_current_frame_number()]);
         }


      if (goal_place)
         {
          CSprite *s = goal_place->sprites[goal_place->state];
          if (! s)
             qDebug() << "! s";

          painter.drawImage (goal_place->element_area.x(), goal_place->element_area.y(),
                             s->images[s->get_current_frame_number()]);
         }


      CSprite *s = hero->sprites[hero->state];
      if (! s)
          qDebug() << "! s";

      painter.drawImage (hero->element_area.x(), hero->element_area.y(),
                         s->images[s->get_current_frame_number()]);


      if (carrot)
         {
          CSprite *sprite_carrot = carrot->sprites[carrot->state];
          if (! sprite_carrot)
             qDebug() << "! sprite_carrot";

          painter.drawImage (carrot->element_area.x(), carrot->element_area.y(),
                             sprite_carrot->images[sprite_carrot->get_current_frame_number()]);
         }


      foreach (CGameElement *ebw, enemy_ball_weapons)
              {
               CSprite *ws = ebw->sprites[ebw->state];
               if (! ws)
                 qDebug() << "! ws";

               painter.drawImage (ebw->element_area.x(), ebw->element_area.y(),
                                  ws->images[ws->get_current_frame_number()]);
              }


//DRAW STATUS

      int x_start = 0;
      int y = area_height - 34;

      for (int i = 0; i < hero->hp; i++)
          {
           painter.drawImage (x_start + status_heart.width(), y, status_heart);
           x_start += status_heart.width();
          }


     if (boss)
        {
         int bx_start = 0;
         int by = 8;

         for (int i = 0; i < boss->hp; i++)
             {
              painter.drawImage (bx_start + status_heart.width(), by, status_heart);
              bx_start += status_heart.width();
             }
         }


     if (current_level->level_goal == LVL_GOAL_COLLECT)
        {
         painter.setPen (Qt::white);
         painter.drawText (area_width - 48, area_height - 20, s_collected_prize_goal);
        }


    if (current_level->level_goal == LVL_GOAL_KILL_BOSS)
        {
         painter.setPen (Qt::white);
         painter.drawText (area_width - 48, area_height - 20, s_killed_goal);
        }


     //end of iteration mode rendering
    painter.end();
    
    QPainter pr (this);
//    pr.drawImage (0, 0, *offscreen_buffer, 0, 0, SOURCE_RES_X, SOURCE_RES_Y);
      pr.drawImage (QRect (0, 0, dest_res_x, dest_res_y), *offscreen_buffer);
    
   }


  if (iteration_mode == ITERATION_TITLE)
     {
 //     qDebug() << "iteration_mode == ITERATION_TITLE";
      QPainter painter (offscreen_buffer);
      
      painter.drawImage (0, 0, title_screen, 0, 0, SOURCE_RES_X, SOURCE_RES_Y);
      
      QPainter pr (this);
      pr.drawImage (QRect (0, 0, dest_res_x, dest_res_y), *offscreen_buffer);
  
//      pr.drawImage (0, 0, *offscreen_buffer);
    
     }

  event->accept();

//qDebug() << "paintEvent -- end";
}


/*
void CGameArea::paintEvent (QPaintEvent * event)
{
//qDebug() << "paintEvent ";

  QImage offscreen_img (SOURCE_RES_X, SOURCE_RES_Y, QImage::Format_ARGB32);
  QPainter offscreen_painter (&offscreen_img);
  QPainter painter (this);

  if (iteration_mode == ITERATION_GAME)
     {

      painter.drawImage (0, 0, background_image);

      if (current_level)
          if (! current_level->abackground.sprite.empty)
              offscreen_painter.drawImage (current_level->abackground.position.x(), current_level->abackground.position.y(),
                                 current_level->abackground.sprite.images[current_level->abackground.sprite.get_current_frame_number()]);


      for (int i = 0; i < fx.count(); i++)
          {
           CGameElement *el = fx[i];
           CSprite *sprite = el->sprites[el->state];

           offscreen_painter.drawImage (el->element_area.x(), el->element_area.y(),
                              sprite->images[sprite->get_current_frame_number()]);
          }


      for (int i = 0; i < collectables.count(); i++)
          {
           CGameElement *e = collectables[i];

           CSprite *e_sprite = e->sprites[e->state];
           offscreen_painter.drawImage (e->element_area.x(), e->element_area.y(),
                              e_sprite->images[e_sprite->get_current_frame_number()]);
          }


      for (int i = 0; i < stars.count(); i++)
          {
           CGameElement *star = stars[i];
           CSprite *star_sprite = star->sprites[star->state];
           if (! star_sprite)
              qDebug() << "! star_sprit";

           offscreen_painter.drawImage (star->element_area.x(), star->element_area.y(),
                              star_sprite->images[star_sprite->get_current_frame_number()]);
          }


      for (int i = 0; i < enemies.count(); i++)
          {
           CGameElement *e = enemies[i];

           CSprite *en1_sprite = e->sprites[e->state];
           if (! en1_sprite)
              qDebug() << "! en1_sprite";

           offscreen_painter.drawImage (e->element_area.x(), e->element_area.y(),
                              en1_sprite->images[en1_sprite->get_current_frame_number()]);
          }


      if (boss)
         {
          CSprite *s = boss->sprites[boss->state];
          if (! s)
             qDebug() << "! s";

          offscreen_painter.drawImage (boss->element_area.x(), boss->element_area.y(),
                             s->images[s->get_current_frame_number()]);
         }


      if (goal_place)
         {
          CSprite *s = goal_place->sprites[goal_place->state];
          if (! s)
             qDebug() << "! s";

          offscreen_painter.drawImage (goal_place->element_area.x(), goal_place->element_area.y(),
                             s->images[s->get_current_frame_number()]);
         }


      CSprite *s = hero->sprites[hero->state];
      if (! s)
          qDebug() << "! s";

      offscreen_painter.drawImage (hero->element_area.x(), hero->element_area.y(),
                         s->images[s->get_current_frame_number()]);


      if (carrot)
         {
          CSprite *sprite_carrot = carrot->sprites[carrot->state];
          if (! sprite_carrot)
             qDebug() << "! sprite_carrot";

          offscreen_painter.drawImage (carrot->element_area.x(), carrot->element_area.y(),
                             sprite_carrot->images[sprite_carrot->get_current_frame_number()]);
         }


      foreach (CGameElement *ebw, enemy_ball_weapons)
              {
               CSprite *ws = ebw->sprites[ebw->state];
               if (! ws)
                 qDebug() << "! ws";

               offscreen_painter.drawImage (ebw->element_area.x(), ebw->element_area.y(),
                                  ws->images[ws->get_current_frame_number()]);
              }


//DRAW STATUS

      int x_start = 0;
      int y = area_height - 34;

      for (int i = 0; i < hero->hp; i++)
          {
           offscreen_painter.drawImage (x_start + status_heart.width(), y, status_heart);
           x_start += status_heart.width();
          }


     if (boss)
        {
         int bx_start = 0;
         int by = 8;

         for (int i = 0; i < boss->hp; i++)
             {
              offscreen_painter.drawImage (bx_start + status_heart.width(), by, status_heart);
              bx_start += status_heart.width();
             }
         }


     if (current_level->level_goal == LVL_GOAL_COLLECT)
        {
         offscreen_painter.setPen (Qt::white);
         offscreen_painter.drawText (area_width - 48, area_height - 20, s_collected_prize_goal);
        }


    if (current_level->level_goal == LVL_GOAL_KILL_BOSS)
        {
         offscreen_painter.setPen (Qt::white);
         offscreen_painter.drawText (area_width - 48, area_height - 20, s_killed_goal);
        }


     //end of iteration mode rendering
    offscreen_painter.end();
   }


  if (iteration_mode == ITERATION_TITLE)
     {
 //     qDebug() << "iteration_mode == ITERATION_TITLE";
      ///QPainter painter (this);
      offscreen_painter.drawImage (0, 0, title_screen);
     }

  QRect dest_rect (1, 1, 1024, 768);

  painter.drawImage (dest_rect,- offscreen_img);
  event->accept();

//qDebug() << "paintEvent -- end";
}
*/

void CGameArea::keyPressEvent (QKeyEvent *event)
{

  if (iteration_mode == ITERATION_TITLE)
     {
      if (event->key() == Qt::Key_Space)
         {
          if (current_titles->current_title == current_titles->list.count() - 1)
             {
              iter_mode_game();
              event->accept();
              return;
             }
           else
               {
                // goto last title
                if (current_titles->current_title != 0)
                   current_titles->current_title = current_titles->list.count() - 1;
                //qDebug() << current_titles->current_title;
                event->accept();
                return;
               }
          }
      }


  if (iteration_mode == ITERATION_GAME)
     {
      if (event->key() == Qt::Key_P)
         {
          if (timer_game.isActive())
             timer_game.stop();
          else
             timer_game.start();
         }

      if (devel_mode)
      if (event->key() == Qt::Key_F1)
         godmode = ! godmode;

      if (event->key() == Qt::Key_Left)
         hero->state = DIRECTION_LEFT;

      if (event->key() == Qt::Key_Right)
         hero->state = DIRECTION_RIGHT;

      if (event->key() == Qt::Key_Up)
         hero->state = DIRECTION_UP;

      if (event->key() == Qt::Key_Down)
         hero->state = DIRECTION_DOWN;

      if (event->key() == Qt::Key_Space)
          if (! carrot)
              create_carrot = true;
  }

  event->accept();
}


void CGameArea::make_carrot()
{
  sound_engine->play_sample (sample_pool[SND_SHOT_CARROT]->get());

  carrot = new CCarrot;

  if (hero->id != EL_ID_HERO_ROCKET)
     carrot->state = hero->state;
  else
      carrot->state = DIRECTION_UP;

  if (carrot->state == DIRECTION_LEFT || carrot->state == DIRECTION_RIGHT)
     {
      carrot->element_area.setWidth (64);
      carrot->element_area.setHeight (32);

      carrot->element_area.moveTo (hero->element_area.x() + hero->element_area.width(),
                                   hero->element_area.y() + hero->element_area.height() / 2);
      }
  else
      {
       carrot->element_area.setWidth (32);
       carrot->element_area.setHeight (64);

       carrot->element_area.moveTo (hero->element_area.x() + hero->element_area.width() / 2,
                                    hero->element_area.y() + hero->element_area.height());
       }


  create_carrot = false;
}


void CGameArea::init_stars()
{
//qDebug() << "CGameArea::init_stars() - start";

  qsrand (QTime::currentTime().msec());

  for (int i = 0; i < current_level->stars_per_wave; i++)
      {
       int x = qrand() % (area_width - 64);
       int y = qrand() % (area_height - 64);

       CStar *s = new CStar();

       s->element_area.moveTo (x, y);
       s->sprites[s->state]->current_frame = qrand() % s->sprites[s->state]->images.count();

       stars.append (s);
      }

//qDebug() << "CGameArea::init_stars() - end";
}


void CGameArea::check_intersections()
{
//qDebug() << "CGameArea::check_intersections() - start";

  foreach (CGameElement *star, stars)
         {
          if (hero->element_area.intersects (star->element_area))
             {
              stars.removeOne (star);
              if (hero->hp < MAX_HP)
                 hero->hp++;

              sound_engine->play_sample (sample_pool[star->snd_collision]->get());
              delete star;
             }
         }


  foreach (CGameElement *e, collectables)
         {
          if (hero->element_area.intersects (e->element_area))
             {
              collectables.removeOne (e);
              collected_prize_goal--;
              s_collected_prize_goal = QString::number (collected_prize_goal);
              sound_engine->play_sample (sample_pool[e->snd_collision]->get());

              delete e;
             }
         }


  foreach (CGameElement *e, enemies)
          {
           foreach (CGameElement *star, stars)
                   {
                    if (e->element_area.intersects (star->element_area))
                      {
                       stars.removeOne (star);
                       delete star;
                      }
                   }


           foreach (CGameElement *cl, collectables)
                   {
                    if (e->element_area.intersects (cl->element_area))
                       {
                        e->val++;
                        collectables.removeOne (cl);
                        delete cl;
                       }
                   }
          }


  if (carrot)
     {
      foreach (CGameElement *e, enemies)
              {
               if (carrot->element_area.intersects (e->element_area))
                  {
                   sound_engine->play_sample (sample_pool[e->snd_collision]->get());

                   enemies.removeOne (e);

                   if (e->fx != -1)
                      {
                      // qDebug() << "create boom fx!";
                       CGameElement *ef = new CFx;
                       //new code
                       ef->sprites[STATE_STAY_AT_PLACE] = sprite_pool[e->fx]->create_copy();
                       //e n c
                       ef->element_area.moveTo (e->element_area.topLeft());
                       fx.append (ef);
                      }

                   delete e;
                   delete carrot;
                   carrot = 0;
                   enemies_to_kill_before_boss--;
                   enemies_to_kill--;
                   s_killed_goal = QString::number (enemies_to_kill_before_boss);
                   //qDebug() << "enemies_to_kill = " << enemies_to_kill;
                   //qDebug() << "enemies_to_kill_before_boss = " << enemies_to_kill_before_boss;

                   return; //'cause we don't need to check beam on NULL here in foreach
                  }
                }

       if (boss) //carrot vs boss
          if (carrot->element_area.intersects (boss->element_area))
             {
              sound_engine->play_sample (sample_pool[boss->snd_collision]->get());

              boss->hp--;

              delete carrot;
              carrot = 0;


              if (boss->hp == 0)
                 {
                  boss->hp = boss->initial_hp;

                  boss = 0;
                 // qDebug() << "BOSS IS DEAD!";

                  if (current_level->level_goal == LVL_GOAL_KILL_BOSS)
                     level_goal_gained = true;
                 }
              }
       }


  foreach (CGameElement *ew, enemy_ball_weapons)
          {
           if (hero->element_area.intersects (ew->element_area))
              {
               enemy_ball_weapons.removeOne (ew);
               delete ew;

               if (! godmode)
                  hero->hp--;

               sound_engine->play_sample (sample_pool[hero->snd_collision]->get());

              // qDebug() << "hero HP = " << hero->hp;
              }
           }


  foreach (CGameElement *e, enemies)
          {
           if (hero->element_area.intersects (e->element_area))
              {
               enemies_to_kill_before_boss--;
               enemies_to_kill--;
               s_killed_goal = QString::number (enemies_to_kill_before_boss);

               collected_prize_goal -= e->val;
               enemies.removeOne (e);
               delete e;
               if (! godmode)
                   hero->hp--;

               sound_engine->play_sample (sample_pool[hero->snd_collision]->get());

             //  qDebug() << "HERO HP" << hero->hp;
              }
          }


  if (boss)
     {
      if (hero->element_area.intersects (boss->element_area))
         {
          boss = 0;
          if (! godmode)
             hero->hp -= 200; //DEATH!!!
       //   qDebug() << "THE HERO IS DEAD" << hero->hp;
         }
     }


  if (goal_place)
     {
      if (hero->element_area.intersects (goal_place->element_area))
          {
           places_checked--;
         //  qDebug() << "places_checked = " << places_checked;
          }
     }

//qDebug() << "CGameArea::check_intersections() - end";
}


void CGameArea::create_enemies()
{
  //qDebug() << "CGameArea::create_enemies()";

  if (enemies.count() != 0)
     return;

  if (current_level->enemy_types.count() == 0)
     return;

  qsrand (QTime::currentTime().msec());

  if (hero->id != EL_ID_HERO_ROCKET)
  for (int i = 0; i < current_level->enemies_per_wave; i++)
      {
       int x = qrand() % (area_width - 64);
       int y = qrand() % (area_height - 240);

       int idx = qrand() % (current_level->enemy_types.count());

       CGameElement *e = current_level->enemy_types[idx]->create_self();

       e->state = 1 + (qrand() % 3);

       e->element_area.moveTo (x, y);

       enemies.append (e);
      }
   else
   for (int i = 0; i < current_level->enemies_per_wave; i++)
      {
       int x = qrand() % (area_width - area_width / 2);
       int y = qrand() % (area_height - area_height / 2);

       int idx = qrand() % (current_level->enemy_types.count());

       CGameElement *e = current_level->enemy_types[idx]->create_self();

       if (qrand() % 2 == 1)
           e->state = DIRECTION_LEFT;
       else
           e->state = DIRECTION_RIGHT;

       e->element_area.moveTo (x, y);

       enemies.append (e);
      }

 //qDebug() << "enemies count = " << enemies.count();
 //qDebug() << "CGameArea::create_enemies() - done";
}


void CGameArea::delete_objects()
{
  foreach (CGameElement *el, collectables)
          {
           collectables.removeOne (el);
           delete el;
          }

  foreach (CGameElement *el, enemies)
          {
           enemies.removeOne (el);
           delete el;
          }

  foreach (CGameElement *el, enemy_ball_weapons)
          {
           enemy_ball_weapons.removeOne (el);
           delete el;
          }

  foreach (CGameElement *el, stars)
          {
           stars.removeOne (el);
           delete el;
          }

   foreach (CGameElement *el, fx)
          {
           stars.removeOne (el);
           delete el;
          }
}


void CGameArea::setup_level (bool death )
{
  qDebug() << "CGameArea::setup_level()";

  iteration_mode = ITERATION_GAME;

  int old_hero_hp = 3;

  if (! current_level)
     current_level = get_level (initial_level);
  else
     {
      QString lnm = current_level->next_level;

      if (death)
         lnm = current_level->death_level;

      if (hero)
          old_hero_hp = hero->hp;

      delete current_level;
      current_level = get_level (lnm);
     }

  if (! current_level)
      return;

  qDebug() << current_level->level_name;

  delete_objects();

  boss = 0;

  background_image = current_level->background;
  level_goal_gained = current_level->goal_gained;
  enemies_to_kill_before_boss = current_level->enemies_to_kill_before_boss;
  collected_prize_goal = current_level->collectable_goal;
  enemies_to_kill = current_level->enemies_to_kill;
  goal_place = current_level->goal_place;


  hero = current_level->hero;

  if (old_hero_hp > hero->hp)
      hero->hp = old_hero_hp;

//  qDebug() << "hero_hp =" << hero->hp;

  if (current_level->level_name == "level01")
     hero->hp = 3;


  places_checked = current_level->places_checked;

//MOVE TO CREATE_OBJECTS
  init_collectables();
  init_stars();
  create_enemies();

  current_titles = current_level->titles;

  qDebug() << "CGameArea::setup_level() - done";
}


void CGameArea::enemies_shoot()
{
//qDebug() << "CGameArea::enemies_shoot() - start";

  foreach (CGameElement *e, enemies)
          {
           if (e->can_shoot)
           if ((qrand() % (e->shoot_factor + 1)) % e->shoot_factor == 0)
              {
               CEnemyBallWeapon *w = new CEnemyBallWeapon;
               w->element_area.moveTo (e->element_area.x(), e->element_area.y());

               if (hero->id != EL_ID_HERO_ROCKET)
                  w->state = e->state;
               else
                   w->state = DIRECTION_DOWN;

               enemy_ball_weapons.append (w);

               sound_engine->play_sample (sample_pool[e->snd_shot]->get());

              }
          }


  if (boss)
     if (qrand() % (boss->shoot_factor + 1) % boss->shoot_factor == 0)
        {
         sound_engine->play_sample (sample_pool[boss->snd_shot]->get());

         CGameElement *w = boss->weapon_shot->create_self();

         w->element_area.moveTo (boss->element_area.x(), boss->element_area.y());
         w->state = DIRECTION_DOWN;
         enemy_ball_weapons.append (w);
       }

//qDebug() << "CGameArea::enemies_shoot() - end";
}


void CGameArea::basic_move (CGameElement *e)
{
//  qDebug() << "CGameArea::basic_move - start";

  int new_x = e->element_area.x();
  int new_y = e->element_area.y();

  switch (e->state)
         {
          case DIRECTION_UP:
                            new_y -= e->speed;
                            break;

          case DIRECTION_DOWN:
                            new_y += e->speed;
                            break;

          case DIRECTION_LEFT:
                            new_x -= e->speed;
                            break;

          case DIRECTION_RIGHT:
                            new_x += e->speed;
                            break;
         };


  if (new_y < 0)
      new_y = 0;

  if (new_x < 0)
     new_x = 0;

  if (new_x + e->element_area.width() > area_width)
      new_x = e->element_area.x();

  if (new_y + e->element_area.height() > area_height)
      new_y = e->element_area.y();

  e->element_area.moveTo (new_x, new_y);

 //qDebug() << "CGameArea::basic_move - end";
}


void CGameArea::create_boss()
{
  if (! boss) //if boss is alive - don't create another one
     {
      boss = current_level->boss;

      //in which part the the game screen the hero is?

      if (hero->element_area.x() < area_width / 2)
         boss->element_area.moveTo (area_width - boss->element_area.width(), 0);
      else
          boss->element_area.moveTo (0, 0);

      sound_engine->play_sample (sample_pool[SND_BOSS_COMING]->get());
     }
}


QString get_path_for_levelname (const QString &levelname)
{
  QString dir = dir_titles;

  QString tfname = dir + levelname + ".en";

  QString locale;

  if (devel_mode)
     locale = locale_override;
  else
      locale = locale_name();

  tfname = change_file_ext (tfname, locale);

  if (! file_exists (tfname))
      tfname = dir + levelname + ".en";

  return tfname;
}



CGameLevel_1::CGameLevel_1(): CGameLevel()
{
  qDebug() << "CGameLevel_1()";

  level_name = "level01";
  next_level = "level02";

  music = new CMusic (dir_snd + "11.ogg", -1);
  titles_music = new CMusic (dir_snd + "18.ogg", -1);


  boss = new CLabBoss;

  hero = new CHero;

  level_goal = LVL_GOAL_COLLECT;

  background.load (dir_pix + "level01-bg.png");

  collectables_per_wave = 13;
  collectable_goal = collectables_per_wave * 3;

  enemies_per_wave = 13;
  enemies_to_kill_before_boss = 21;

  titles = new CTitles;
  titles->load_from_file (get_path_for_levelname ("01"));

/*  abackground.load ("kolba01.png");
  abackground.animatype = ANIMATYPE_ROTATE;

  abackground.position.setX (400);
  abackground.position.setY (100);
  abackground.angle = 0;
*/

  collectable_types.append (new CAnimal01);
  collectable_types.append (new CAnimal02);
  collectable_types.append (new CAnimal03);

 // qDebug() << "collectable_types.count = " << collectable_types.count();

  enemy_types.append (new CVivisector01);
  enemy_types.append (new CVivisector02);
}


CGameLevel_2::CGameLevel_2(): CGameLevel()
{
  qDebug() << "CGameLevel_2()";

  level_name = "level02";
  next_level = "level03";

  music = new CMusic (dir_snd + "12.ogg", -1);

  titles_music = new CMusic (dir_snd + "18.ogg", -1);

  boss = new CToughBoss;
  hero = new CHero;

  background.load (dir_pix + "level02-bg.png");

  level_goal = LVL_GOAL_COLLECT;


  enemies_per_wave = 6;

  collectables_per_wave = 13;
  collectable_goal = collectables_per_wave * 3;
  enemies_to_kill_before_boss = 15;

  titles = new CTitles;
  titles->load_from_file (get_path_for_levelname ("02"));

  collectable_types.append (new CNuke);

  enemy_types.append (new CEnemyNo1);
  enemy_types.append (new CEnemyNo2);
}


CGameLevel_3::CGameLevel_3(): CGameLevel()
{
  qDebug() << "CGameLevel_3()";

  level_name = "level03";
  next_level = "level04";

  music = new CMusic (dir_snd + "10.ogg", -1);
  titles_music = new CMusic (dir_snd + "18.ogg", -1);

  boss = new CToughBoss;
  hero = new CHero;

  goal_place = new CRocket;

  background.load (dir_pix + "level03-bg.png");

  level_goal = LVL_GOAL_SPOTPLACE;

  collectables_per_wave = 1;
  collectable_goal = collectables_per_wave * 5;

  enemies_to_kill_before_boss = 155;

  enemies_per_wave = 7;

  places_checked = 1;

  titles = new CTitles;
  titles->load_from_file (get_path_for_levelname ("03"));

  enemy_types.append (new CEnemyNo1);
  enemy_types.append (new CEnemyNo2);
}


CGameLevel_4::CGameLevel_4(): CGameLevel()
{
  qDebug() << "CGameLevel_4()";

  level_name = "level04";
  next_level = "level05";

  death_level = "luna";

  music = new CMusic (dir_snd + "11.ogg", -1);
  titles_music = new CMusic (dir_snd + "18.ogg", -1);

  boss = new CDemonBoss;
  hero = new CHeroRocket;

  background.load (dir_pix + "level04-bg.png");

  level_goal = LVL_GOAL_KILL_BOSS;

  collectables_per_wave = 0;
  collectable_goal = 0;

  enemies_per_wave = 15;

  enemies_to_kill_before_boss = 23;
  stars_per_wave = 1;

  places_checked = 1;

  abackground.load ("earth.png");
  abackground.animatype = ANIMATYPE_MOVE_LR;

  abackground.position.setX (RES_X); //FIXME: hardcode!
  abackground.position.setY (100);

  titles = new CTitles;
  titles->load_from_file (get_path_for_levelname ("04"));

  enemy_types.append (new CUFO01);
  enemy_types.append (new CUFO02);
}


CGameLevel_6::CGameLevel_6(): CGameLevel()
{
  qDebug() << "CGameLevel_6()";

  level_name = "level06";
  next_level = "level01";

  goal_gained = true;

  music = new CMusic (dir_snd + "15.ogg", -1);
  titles_music = new CMusic (dir_snd + "15.ogg", -1);


  boss = new CToughBoss;
  hero = new CHeroRocket;

  background.load (dir_pix + "level04-bg.png");

  level_goal = LVL_GOAL_KILL;

  collectables_per_wave = 0;
  collectable_goal = 0;
  enemies_to_kill_before_boss = 15;

  places_checked = 1;

  enemies_to_kill = 4;

  titles = new CTitles;
  titles->load_from_file (get_path_for_levelname ("06"));

  enemy_types.append (new CUFO01);

 // qDebug() << "CGameLevel_6() - ok";
}


CGameLevel::~CGameLevel()
{
  qDebug() << "~CGameLevel()";

  if (goal_place)
     delete goal_place;

  if (boss)
     delete boss;

  if (hero)
      delete hero;

  if (music)
     delete music;

  if (titles_music)
     delete titles_music;

  foreach (CGameElement *e, collectable_types)
          delete e;

  foreach (CGameElement *e, enemy_types)
          delete e;

  delete titles;
}


CGameLevel::CGameLevel()
{
  qDebug() << "CGameLevel::CGameLevel()";

  boss = 0;
  titles = 0;
  music = 0;
  titles_music = 0;
  hero = 0;

  goal_gained = 0;

  death_level = "level_gameover";

  places_checked = 0;
  goal_place = 0;
  enemies_to_kill = 0;
  collectable_goal = 13;
  enemies_to_kill_before_boss = 13;
  enemies_per_wave = 7;
  stars_per_wave = 3;
  collectables_per_wave = 13;
}


CGameLevel* CGameArea::get_level (const QString &lname)
{
  if (lname == "level01")
     return new CGameLevel_1;

  if (lname == "level02")
     return new CGameLevel_2;

  if (lname == "level03")
     return new CGameLevel_3;

  if (lname == "level04")
     return new CGameLevel_4;

  if (lname == "level05")
     return new CGameLevel_5;

  if (lname == "level06")
     return new CGameLevel_6;

  if (lname == "luna")
     return new CGameLevel_luna;

  if (lname == "level_gameover")
     return new CGameLevel_gameover;

  if (lname == "after-luna")
     return new CGameLevel_after_luna;

  return NULL;
}


void CGameArea::iter_title()
{
  //qDebug() << "CGameArea::iter_title()";

  CTitle *t = current_titles->list[current_titles->current_title];

  if (t->duration == 0)
     {
      if (++current_titles->current_title >= current_titles->list.count())
          current_titles->current_title--;

      t = current_titles->list[current_titles->current_title];
      return;
     }


  if (t->is_image)
      title_screen = t->image;
  else
      {
       QImage img (SOURCE_RES_X, SOURCE_RES_Y, QImage::Format_RGB32);
       img.fill (Qt::black);

       QPainter painter (&img);

       painter.setPen (t->color);

       QFont f ("Sans", 24);
       painter.setFont (f);

       QRect r;
       r.setTopLeft (t->pos);
       r.setWidth (600);
       r.setHeight (300);

       painter.drawText (r, Qt::AlignLeft | Qt::TextWordWrap, t->text);
       title_screen = img;
      }

  t->duration--;

  update();
}


void CGameArea::iter_mode_game()
{
  iteration_mode = ITERATION_GAME;

  timer_title.stop();

  sound_engine->play_music (current_level->music);

  timer_game.start();
}


void CGameArea::iter_mode_title()
{
  iteration_mode = ITERATION_TITLE;

  timer_game.stop();

  sound_engine->play_music (current_level->titles_music);

  timer_title.start();
}


CTitle::CTitle (const QString &val, int seconds, int x, int y, bool image_mode, QColor cl)
{
  is_image = image_mode;

  color = cl;

  if (is_image)
     image.load (dir_pix + val);
  else
      {
       text = val;
       text = text.replace (QString("<br>"), QString ("\n"));
      }

  pos.setX (x);
  pos.setY (y);

  duration = seconds;
}


bool CTitles::load_from_file (const QString &fname)
{
  QStringList sl = qstring_load (fname, "UTF-8").split ("\n");

  for (int i = 0; i < sl.count(); i++)
      {
       QStringList two_parts = sl[i].split ('"');
       if (two_parts.count() < 2)
          continue;

       QStringList sl_operator = two_parts[0].split(" ");
       QString s_keyword = sl_operator[0];
       QString s_duration = sl_operator[1];
       QString s_x = sl_operator[2];
       QString s_y = sl_operator[3];
       QColor cl (Qt::white);

       //qDebug() << "sl_operator.count() = " << sl_operator.count();
       //qDebug() << sl_operator.at(4);

       if (sl_operator.count() == 6)
          cl = QColor (sl_operator.at(4));

       //qDebug() << "cl.name()" << cl.name();

       QString s_operand = two_parts[1];
/*
       qDebug() << s_keyword;
       qDebug() << s_duration;
       qDebug() << s_x;
       qDebug() << s_y;
       qDebug() << s_operand;
*/
       bool img_mode = false;

       if (s_keyword == "image")
         img_mode = true;

       CTitle *t = new CTitle (s_operand, s_duration.toInt(), s_x.toInt(), s_y.toInt(), img_mode, cl);

       list.append (t);
      }

  return true;
}


CTitles::CTitles()
{
  current_title = 0;
}


CTitles::~CTitles()
{
  foreach (CTitle *t, list)
          delete t;
}


void CGameArea::init_collectables()
{
//qDebug() << "CGameArea::init_collectables() - start";

  if (collectables.count() != 0)
     return;

  if (current_level->collectable_types.count() == 0)
     return;

  qsrand (QTime::currentTime().msec());

  for (int i = 0; i < current_level->collectables_per_wave; i++)
      {
       int x = qrand() % (area_width - 64);
       int y = qrand() % (area_height - 64);

       //int idx = qrand() % (current_level->collectable_types.count());
       int idx = qrand() % (current_level->collectable_types.count());

       //qDebug() << "idx = " << idx;

       CGameElement *e = current_level->collectable_types[idx]->create_self();

       e->element_area.moveTo (x, y);

       collectables.append (e);
      }

//qDebug() << "CGameArea::init_collectables() - end";
}


void CAnimabackground::load (const QString &fname)
{
  //image.load (dir_pix + fname);
  //render_image = image;
  sprite.add_image (dir_pix + fname);
 // qDebug() << fname;
}


void CGameArea::init_sprite_pool()
{
  for (int i = 0; i < MAX_SPRITES; i++)
       sprite_pool[i] = 0;

  CSprite *s = new CSprite;
  s->add_image (dir_pix + "yourik-up01.png");
  s->add_image (dir_pix + "yourik-up02.png");
  sprite_pool[SPR_HERO_DIR_UP] = s;

  s = new CSprite;
  s->add_image (dir_pix + "yourik-down01.png");
  s->add_image (dir_pix + "yourik-down02.png");
  sprite_pool[SPR_HERO_DIR_DOWN] = s;

  s = new CSprite;
  s->add_image (dir_pix + "yourik-left01.png");
  s->add_image (dir_pix + "yourik-left02.png");
  sprite_pool[SPR_HERO_DIR_LEFT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "yourik-right01.png");
  s->add_image (dir_pix + "yourik-right02.png");
  sprite_pool[SPR_HERO_DIR_RIGHT] = s;


  s = new CSprite;
  s->add_image (dir_pix + "star01.png");
  s->add_image (dir_pix + "star02.png");
  s->add_image (dir_pix + "star03.png");
  s->add_image (dir_pix + "star04.png");
  s->add_image (dir_pix + "star05.png");
  s->add_image (dir_pix + "star06.png");
  sprite_pool[SPR_STAR_STAY_AT_PLACE] = s;


  s = new CSprite;
  s->add_image (dir_pix + "beam01.png");
  s->add_image (dir_pix + "beam02.png");
  s->add_image (dir_pix + "beam03.png");
  s->add_image (dir_pix + "beam04.png");
  sprite_pool[SPR_BEAM_DIR_RIGHT] = s;


  s = new CSprite;
  s->add_image (dir_pix + "beam01.png");
  s->add_image (dir_pix + "beam02.png");
  s->add_image (dir_pix + "beam03.png");
  s->add_image (dir_pix + "beam04.png");
  sprite_pool[SPR_BEAM_DIR_LEFT] = s;


  s = new CSprite;
  s->add_image (dir_pix + "beam-up01.png");
  s->add_image (dir_pix + "beam-up02.png");
  s->add_image (dir_pix + "beam-up03.png");
  s->add_image (dir_pix + "beam-up04.png");
  sprite_pool[SPR_BEAM_DIR_UP] = s;


  s = new CSprite;
  s->add_image (dir_pix + "beam-up01.png");
  s->add_image (dir_pix + "beam-up02.png");
  s->add_image (dir_pix + "beam-up03.png");
  s->add_image (dir_pix + "beam-up04.png");
  sprite_pool[SPR_BEAM_DIR_DOWN] = s;


  s = new CSprite;
  s->add_image (dir_pix + "ufo01-01.png");
  s->add_image (dir_pix + "ufo01-02.png");
  s->add_image (dir_pix + "ufo01-03.png");
  s->add_image (dir_pix + "ufo01-04.png");
  s->add_image (dir_pix + "ufo01-05.png");
  s->add_image (dir_pix + "ufo01-06.png");
  s->add_image (dir_pix + "ufo01-07.png");
  s->add_image (dir_pix + "ufo01-08.png");
  s->add_image (dir_pix + "ufo01-09.png");
  s->add_image (dir_pix + "ufo01-10.png");
  s->add_image (dir_pix + "ufo01-11.png");
  sprite_pool[SPR_UFO01_DIR_LEFT] = s;


  s = new CSprite;
  s->add_image (dir_pix + "ufo01-01.png");
  s->add_image (dir_pix + "ufo01-02.png");
  s->add_image (dir_pix + "ufo01-03.png");
  s->add_image (dir_pix + "ufo01-04.png");
  s->add_image (dir_pix + "ufo01-05.png");
  s->add_image (dir_pix + "ufo01-06.png");
  s->add_image (dir_pix + "ufo01-07.png");
  s->add_image (dir_pix + "ufo01-08.png");
  s->add_image (dir_pix + "ufo01-09.png");
  s->add_image (dir_pix + "ufo01-10.png");
  s->add_image (dir_pix + "ufo01-11.png");
  sprite_pool[SPR_UFO01_DIR_RIGHT] = s;


  s = new CSprite;
  s->cycles_per_frame = 5;
  s->add_image (dir_pix + "ufo02-01.png");
  s->add_image (dir_pix + "ufo02-02.png");
  s->add_image (dir_pix + "ufo02-03.png");
  s->add_image (dir_pix + "ufo02-04.png");
  s->add_image (dir_pix + "ufo02-05.png");
  s->add_image (dir_pix + "ufo02-06.png");
  s->add_image (dir_pix + "ufo02-07.png");
  s->add_image (dir_pix + "ufo02-08.png");
  s->add_image (dir_pix + "ufo02-09.png");
  s->add_image (dir_pix + "ufo02-10.png");
  sprite_pool[SPR_UFO02_DIR_LEFT] = s;


  s = new CSprite;
  s->cycles_per_frame = 10;
  s->add_image (dir_pix + "ufo02-01.png");
  s->add_image (dir_pix + "ufo02-02.png");
  s->add_image (dir_pix + "ufo02-03.png");
  s->add_image (dir_pix + "ufo02-04.png");
  s->add_image (dir_pix + "ufo02-05.png");
  s->add_image (dir_pix + "ufo02-06.png");
  s->add_image (dir_pix + "ufo02-07.png");
  s->add_image (dir_pix + "ufo02-08.png");
  s->add_image (dir_pix + "ufo02-09.png");
  s->add_image (dir_pix + "ufo02-10.png");
  sprite_pool[SPR_UFO02_DIR_RIGHT] = s;


  s = new CSprite;
  s->add_image (dir_pix + "enemy01-up01.png");
  s->add_image (dir_pix + "enemy01-up02.png");
  sprite_pool[SPR_ENEMY01_DIR_UP] = s;

  s = new CSprite;
  s->add_image (dir_pix + "enemy01-down01.png");
  s->add_image (dir_pix + "enemy01-down02.png");
  sprite_pool[SPR_ENEMY01_DIR_DOWN] = s;

  s = new CSprite;
  s->add_image (dir_pix + "enemy01-left01.png");
  s->add_image (dir_pix + "enemy01-left02.png");
  sprite_pool[SPR_ENEMY01_DIR_LEFT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "enemy01-right01.png");
  s->add_image (dir_pix + "enemy01-right02.png");
  sprite_pool[SPR_ENEMY01_DIR_RIGHT] = s;


  s = new CSprite;
  s->add_image (dir_pix + "enemy_ball_weapon.png");
  sprite_pool[SPR_ENEMY_BALL_WEAPON_DIR_UP] = s;

  s = new CSprite;
  s->add_image (dir_pix + "enemy_ball_weapon.png");
  sprite_pool[SPR_ENEMY_BALL_WEAPON_DIR_DOWN] = s;

  s = new CSprite;
  s->add_image (dir_pix + "enemy_ball_weapon.png");
  sprite_pool[SPR_ENEMY_BALL_WEAPON_DIR_LEFT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "enemy_ball_weapon.png");
  sprite_pool[SPR_ENEMY_BALL_WEAPON_DIR_RIGHT] = s;


  s = new CSprite;
  s->add_image (dir_pix + "boss01.png");
  s->add_image (dir_pix + "boss02.png");
  s->add_image (dir_pix + "boss03.png");
  s->add_image (dir_pix + "boss04.png");
  s->add_image (dir_pix + "boss05.png");
  s->add_image (dir_pix + "boss06.png");
  sprite_pool[SPR_TOUGH_BOSS_DIR_LEFT] = s;


  s = new CSprite;
  s->add_image (dir_pix + "boss01.png");
  s->add_image (dir_pix + "boss02.png");
  s->add_image (dir_pix + "boss03.png");
  s->add_image (dir_pix + "boss04.png");
  s->add_image (dir_pix + "boss05.png");
  s->add_image (dir_pix + "boss06.png");
  sprite_pool[SPR_TOUGH_BOSS_DIR_RIGHT] = s;


  s = new CSprite;
  s->cycles_per_frame = 10;
  s->add_image (dir_pix + "lab-boss-left01.png");
  s->add_image (dir_pix + "lab-boss-left02.png");
  s->add_image (dir_pix + "lab-boss-left03.png");
  s->add_image (dir_pix + "lab-boss-left04.png");
  s->add_image (dir_pix + "lab-boss-left05.png");
  s->add_image (dir_pix + "lab-boss-left06.png");
  sprite_pool[SPR_LAB_BOSS_DIR_LEFT] = s;

  s = new CSprite;
  s->cycles_per_frame = 10;
  s->add_image (dir_pix + "lab-boss-left01.png");
  s->add_image (dir_pix + "lab-boss-left02.png");
  s->add_image (dir_pix + "lab-boss-left03.png");
  s->add_image (dir_pix + "lab-boss-left04.png");
  s->add_image (dir_pix + "lab-boss-left05.png");
  s->add_image (dir_pix + "lab-boss-left06.png");
  sprite_pool[SPR_LAB_BOSS_DIR_RIGHT] = s;


  s = new CSprite;
  s->add_image (dir_pix + "greenman-up01.png");
  s->add_image (dir_pix + "greenman-up02.png");
  sprite_pool[SPR_ENEMY02_DIR_UP] = s;

  s = new CSprite;
  s->add_image (dir_pix + "greenman-down01.png");
  s->add_image (dir_pix + "greenman-down02.png");
  sprite_pool[SPR_ENEMY02_DIR_DOWN] = s;

  s = new CSprite;
  s->add_image (dir_pix + "greenman-left01.png");
  s->add_image (dir_pix + "greenman-left02.png");
  sprite_pool[SPR_ENEMY02_DIR_LEFT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "greenman-right01.png");
  s->add_image (dir_pix + "greenman-right02.png");
  sprite_pool[SPR_ENEMY02_DIR_RIGHT] = s;


  s = new CSprite;
  s->add_image (dir_pix + "carrot-up.png");
  sprite_pool[SPR_CARROT_DIR_UP] = s;

  s = new CSprite;
  s->add_image (dir_pix + "carrot-down.png");
  sprite_pool[SPR_CARROT_DIR_DOWN] = s;

  s = new CSprite;
  s->add_image (dir_pix + "carrot-left.png");
  sprite_pool[SPR_CARROT_DIR_LEFT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "carrot-right.png");
  sprite_pool[SPR_CARROT_DIR_RIGHT] = s;


  s = new CSprite;
  s->add_image (dir_pix + "animal01.png");
  sprite_pool[SPR_ANIMAL01] = s;

  s = new CSprite;
  s->add_image (dir_pix + "animal02.png");
  sprite_pool[SPR_ANIMAL02] = s;

  s = new CSprite;
  s->add_image (dir_pix + "animal03.png");
  sprite_pool[SPR_ANIMAL03] = s;


  s = new CSprite;
//  s->cycles_per_frame = 100;
  s->add_image (dir_pix + "lab-enemy01-up01.png");
  s->add_image (dir_pix + "lab-enemy01-up02.png");
  sprite_pool[SPR_VIVI01_DIR_UP] = s;

  s = new CSprite;
//  s->cycles_per_frame = 100;
  s->add_image (dir_pix + "lab-enemy01-down01.png");
  s->add_image (dir_pix + "lab-enemy01-down02.png");
  sprite_pool[SPR_VIVI01_DIR_DOWN] = s;

  s = new CSprite;
//  s->cycles_per_frame = 100;
  s->add_image (dir_pix + "lab-enemy01-left01.png");
  s->add_image (dir_pix + "lab-enemy01-left02.png");
  sprite_pool[SPR_VIVI01_DIR_LEFT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "lab-enemy01-right01.png");
  s->add_image (dir_pix + "lab-enemy01-right02.png");
  sprite_pool[SPR_VIVI01_DIR_RIGHT] = s;


  s = new CSprite;
  s->add_image (dir_pix + "lab-enemy02-up01.png");
  s->add_image (dir_pix + "lab-enemy02-up02.png");
  sprite_pool[SPR_VIVI02_DIR_UP] = s;

  s = new CSprite;
  s->add_image (dir_pix + "lab-enemy02-down01.png");
  s->add_image (dir_pix + "lab-enemy02-down02.png");
  sprite_pool[SPR_VIVI02_DIR_DOWN] = s;

  s = new CSprite;
  s->add_image (dir_pix + "lab-enemy02-left01.png");
  s->add_image (dir_pix + "lab-enemy02-left02.png");
  sprite_pool[SPR_VIVI02_DIR_LEFT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "lab-enemy02-right01.png");
  s->add_image (dir_pix + "lab-enemy02-right02.png");
  sprite_pool[SPR_VIVI02_DIR_RIGHT] = s;



  s = new CSprite;
  s->add_image (dir_pix + "nuke.png");
  sprite_pool[SPR_NUKE_STAY_AT_PLACE] = s;


  s = new CSprite;
  s->cycles_per_frame = 5;
  s->add_image (dir_pix + "knife01.png");
  s->add_image (dir_pix + "knife02.png");
  s->add_image (dir_pix + "knife03.png");
  s->add_image (dir_pix + "knife04.png");
  s->add_image (dir_pix + "knife05.png");
  s->add_image (dir_pix + "knife06.png");
  s->add_image (dir_pix + "knife07.png");
  s->add_image (dir_pix + "knife08.png");
  s->add_image (dir_pix + "knife09.png");
  s->add_image (dir_pix + "knife10.png");
  s->add_image (dir_pix + "knife11.png");
  sprite_pool[SPR_KNIFE_DIR_DOWN] = s;


  s = new CSprite;
  s->cycles_per_frame = 10;
  s->add_image (dir_pix + "wrench-01.png");
  s->add_image (dir_pix + "wrench-02.png");
  s->add_image (dir_pix + "wrench-03.png");
  s->add_image (dir_pix + "wrench-04.png");
  s->add_image (dir_pix + "wrench-05.png");
  s->add_image (dir_pix + "wrench-06.png");
  s->add_image (dir_pix + "wrench-07.png");
  s->add_image (dir_pix + "wrench-08.png");
  s->add_image (dir_pix + "wrench-09.png");
  s->add_image (dir_pix + "wrench-10.png");
  s->add_image (dir_pix + "wrench-12.png");
  s->add_image (dir_pix + "wrench-13.png");
  sprite_pool[SPR_WRENCH_DIR_DOWN] = s;


  s = new CSprite;
  s->add_image (dir_pix + "birbalet.png");
  sprite_pool[SPR_ROCKET_STAY_AT_PLACE] = s;


  s = new CSprite;
  s->add_image (dir_pix + "birbalet-up01.png");
  s->add_image (dir_pix + "birbalet-up02.png");
  s->add_image (dir_pix + "birbalet-up03.png");
  sprite_pool[SPR_HERO_ROCKET_DIR_UP] = s;

  s = new CSprite;
  s->add_image (dir_pix + "birbalet-up01.png");
  s->add_image (dir_pix + "birbalet-up02.png");
  s->add_image (dir_pix + "birbalet-up03.png");
  sprite_pool[SPR_HERO_ROCKET_DIR_DOWN] = s;

  s = new CSprite;
  s->add_image (dir_pix + "birbalet-up01.png");
  s->add_image (dir_pix + "birbalet-up02.png");
  s->add_image (dir_pix + "birbalet-up03.png");
  sprite_pool[SPR_HERO_ROCKET_DIR_LEFT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "birbalet-up01.png");
  s->add_image (dir_pix + "birbalet-up02.png");
  s->add_image (dir_pix + "birbalet-up03.png");
  sprite_pool[SPR_HERO_ROCKET_DIR_RIGHT] = s;


  s = new CSprite;
  s->add_image (dir_pix + "demon-left.png");
  sprite_pool[SPR_DEMON_DIR_LEFT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "demon-right.png");
  sprite_pool[SPR_DEMON_DIR_RIGHT] = s;


  s = new CSprite;
  s->add_image (dir_pix + "robotank-right-01.png");
  s->add_image (dir_pix + "robotank-right-02.png");
  sprite_pool[SPR_ROBOTANK_DIR_RIGHT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "robotank-left-01.png");
  s->add_image (dir_pix + "robotank-left-02.png");
  sprite_pool[SPR_ROBOTANK_DIR_LEFT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "robotank-up-01.png");
  s->add_image (dir_pix + "robotank-up-02.png");
  s->add_image (dir_pix + "robotank-up-03.png");
  s->add_image (dir_pix + "robotank-up-04.png");
  sprite_pool[SPR_ROBOTANK_DIR_UP] = s;

  s = new CSprite;
  s->add_image (dir_pix + "robotank-down-01.png");
  s->add_image (dir_pix + "robotank-down-02.png");
  sprite_pool[SPR_ROBOTANK_DIR_DOWN] = s;

  s = new CSprite;
  s->add_image (dir_pix + "robot-boss-01.png");
  s->add_image (dir_pix + "robot-boss-02.png");
  s->add_image (dir_pix + "robot-boss-03.png");
  s->add_image (dir_pix + "robot-boss-04.png");
  s->add_image (dir_pix + "robot-boss-05.png");
  s->add_image (dir_pix + "robot-boss-06.png");
  sprite_pool[SPR_ROBOBOSS_DIR_LEFT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "robot-boss-01.png");
  s->add_image (dir_pix + "robot-boss-02.png");
  s->add_image (dir_pix + "robot-boss-03.png");
  s->add_image (dir_pix + "robot-boss-04.png");
  s->add_image (dir_pix + "robot-boss-05.png");
  s->add_image (dir_pix + "robot-boss-06.png");
  sprite_pool[SPR_ROBOBOSS_DIR_RIGHT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "zombie-up-01.png");
  s->add_image (dir_pix + "zombie-up-02.png");
  s->add_image (dir_pix + "zombie-up-03.png");
  s->add_image (dir_pix + "zombie-up-04.png");
  sprite_pool[SPR_ZOMBIE_DIR_UP] = s;

  s = new CSprite;
  s->add_image (dir_pix + "zombie-down-01.png");
  s->add_image (dir_pix + "zombie-down-02.png");
  s->add_image (dir_pix + "zombie-down-03.png");
  s->add_image (dir_pix + "zombie-down-04.png");
  sprite_pool[SPR_ZOMBIE_DIR_DOWN] = s;

  s = new CSprite;
  s->add_image (dir_pix + "zombie-left-01.png");
  s->add_image (dir_pix + "zombie-left-02.png");
  s->add_image (dir_pix + "zombie-left-03.png");
  s->add_image (dir_pix + "zombie-left-04.png");
  sprite_pool[SPR_ZOMBIE_DIR_LEFT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "zombie-right-01.png");
  s->add_image (dir_pix + "zombie-right-02.png");
  s->add_image (dir_pix + "zombie-right-03.png");
  s->add_image (dir_pix + "zombie-right-04.png");
  sprite_pool[SPR_ZOMBIE_DIR_RIGHT] = s;

//

  s = new CSprite;
  s->add_image (dir_pix + "zombie-02-up-01.png");
  s->add_image (dir_pix + "zombie-02-up-02.png");
  s->add_image (dir_pix + "zombie-02-up-03.png");
  sprite_pool[SPR_ZOMBIE02_DIR_UP] = s;

  s = new CSprite;
  s->add_image (dir_pix + "zombie-02-down-01.png");
  s->add_image (dir_pix + "zombie-02-down-02.png");
  s->add_image (dir_pix + "zombie-02-down-03.png");
  s->add_image (dir_pix + "zombie-02-down-04.png");
  sprite_pool[SPR_ZOMBIE02_DIR_DOWN] = s;

  s = new CSprite;
  s->add_image (dir_pix + "zombie-02-left-01.png");
  s->add_image (dir_pix + "zombie-02-left-02.png");
  sprite_pool[SPR_ZOMBIE02_DIR_LEFT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "zombie-02-right-01.png");
  s->add_image (dir_pix + "zombie-02-right-02.png");
  sprite_pool[SPR_ZOMBIE02_DIR_RIGHT] = s;

  s = new CSprite;
  s->add_image (dir_pix + "katafalk-spot.png");
  sprite_pool[SPR_KATAFALK_STAY_AT_PLACE] = s;

  s = new CSprite;
  s->cycles_per_frame = 40;
  s->add_image (dir_pix + "boom-ufo-01.png");
  s->add_image (dir_pix + "boom-ufo-02.png");
  s->add_image (dir_pix + "boom-ufo-03.png");
  s->add_image (dir_pix + "boom-ufo-04.png");
  sprite_pool[SPR_BOOM_UFO_STAY_AT_PLACE] = s;

/*
  s = new CSprite;
  s->cycles_per_frame = 40;
  s->add_image (dir_pix + "vivi-death-01.png");
  s->add_image (dir_pix + "vivi-death-02.png");
  s->add_image (dir_pix + "vivi-death-03.png");
  s->add_image (dir_pix + "vivi-death-04.png");
  s->add_image (dir_pix + "vivi-death-05.png");
  s->add_image (dir_pix + "vivi-death-06.png");
  s->add_image (dir_pix + "vivi-death-07.png");
  s->add_image (dir_pix + "vivi-death-08.png");
  s->add_image (dir_pix + "vivi-death-09.png");
  s->add_image (dir_pix + "vivi-death-10.png");
  s->add_image (dir_pix + "vivi-death-11.png");
  s->add_image (dir_pix + "vivi-death-12.png");
  s->add_image (dir_pix + "vivi-death-13.png");
  s->add_image (dir_pix + "vivi-death-14.png");
  s->add_image (dir_pix + "vivi-death-15.png");
  sprite_pool[SPR_BOOM_VIVI_STAY_AT_PLACE] = s;
*/
}


void CGameArea::done_sprite_pool()
{
  for (int i = 0; i < MAX_SPRITES; i++)
      {
       if (sprite_pool[i])
         delete sprite_pool[i];
      }
}


CGameLevel_5::CGameLevel_5(): CGameLevel()
{
  qDebug() << "CGameLevel_5()";

  level_name = "level05";
  next_level = "level06";

  music = new CMusic (dir_snd + "12.ogg", -1);
  titles_music = new CMusic (dir_snd + "18.ogg", -1);

  boss = new CRoboBoss;
  hero = new CHero;

  background.load (dir_pix + "level05-bg.png");

  level_goal = LVL_GOAL_KILL_BOSS;

  collectables_per_wave = 0;
  collectable_goal = 0;
  stars_per_wave = 2;
  enemies_per_wave = 7;

  enemies_to_kill_before_boss = 27;

  places_checked = 1;

  enemies_to_kill = 14;

  titles = new CTitles;
  titles->load_from_file (get_path_for_levelname ("05"));

  enemy_types.append (new CRobotank);
  enemy_types.append (new CEnemyNo2);
}



CGameLevel_luna::CGameLevel_luna(): CGameLevel()
{
  qDebug() << "CGameLevel_luna()";

  level_name = "luna";

  next_level = "after-luna";

  music = new CMusic (dir_snd + "17.ogg", -1);
  titles_music = new CMusic (dir_snd + "12.ogg", -1);

  boss = new CRoboBoss;
  hero = new CHero;

  hero->element_area.moveTo (area_width - 64, area_height - 64);


  background.load (dir_pix + "luna-bg.png");

  goal_place = new CKatafalk;

  level_goal = LVL_GOAL_SPOTPLACE;

  collectables_per_wave = 0;
  collectable_goal = 0;
  enemies_to_kill_before_boss = 666;

  enemies_per_wave = 30;

  places_checked = 1;

  enemies_to_kill = 14;

  titles = new CTitles;
  titles->load_from_file (get_path_for_levelname ("luna"));

  //qDebug() << "items count = " << titles->list.count();

//FIXME if comment it, game crashes
 // collectable_types.append (new CNuke);

  //qDebug() << "collectable_types.count = " << collectable_types.count();


  enemy_types.append (new CZombie);
  enemy_types.append (new CZombie02);
}


CGameLevel_gameover::CGameLevel_gameover(): CGameLevel()
{
  qDebug() << "CGameLevel_gameover()";

  level_name = "gameover";
  next_level = "level01";

  goal_gained = true;

  music = new CMusic (dir_snd + "level02-music.ogg", -1);

  boss = new CToughBoss;
  hero = new CHeroRocket;

  background.load (dir_pix + "level04-bg.png");

  level_goal = LVL_GOAL_KILL;

  collectables_per_wave = 0;
  collectable_goal = 0;
  enemies_to_kill_before_boss = 15;

  places_checked = 1;

  enemies_to_kill = 4;

  titles = new CTitles;
  titles->load_from_file (get_path_for_levelname ("gameover"));


  enemy_types.append (new CUFO01);

//  qDebug() << "CGameLevel_gameover() - ok";
}


CGameLevel_after_luna::CGameLevel_after_luna(): CGameLevel()
{
  qDebug() << "CGameLevel_after_luna";

  level_name = "after-luna";
  next_level = "level01";

  goal_gained = true;

  music = new CMusic (dir_snd + "level02-music.ogg", -1);
  titles_music = new CMusic (dir_snd + "05.ogg", -1);

  //boss = new CToughBoss;
  hero = new CHeroRocket;

  background.load (dir_pix + "level04-bg.png");

  level_goal = LVL_GOAL_KILL;

  collectables_per_wave = 0;
  collectable_goal = 0;
  enemies_to_kill_before_boss = 15;

  places_checked = 1;

  enemies_to_kill = 4;

  titles = new CTitles;
  titles->load_from_file (get_path_for_levelname ("after-luna"));


  enemy_types.append (new CUFO01);

//  qDebug() << "CGameLevel_gameover() - ok";
}


void CGameArea::done_sample_pool()
{
  for (int i = 0; i < MAX_SOUNDS; i++)
      {
       if (sample_pool[i])
         delete sample_pool[i];
      }
}


void CGameArea::init_sample_pool()
{
  for (int i = 0; i < MAX_SOUNDS; i++)
       sample_pool[i] = 0;

  CSampleList *s = new CSampleList;

  s->samples.append (new CSample (dir_snd + "boss-coming-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "boss-coming-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "boss-coming-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "boss-coming-04.ogg", 0));
  s->samples.append (new CSample (dir_snd + "boss-coming-05.ogg", 0));
  s->samples.append (new CSample (dir_snd + "boss-coming-06.ogg", 0));

  sample_pool[SND_BOSS_COMING] = s;

  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "shot-carrot-01.ogg", 0));
  sample_pool[SND_SHOT_CARROT] = s;

  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "nuke-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "nuke-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "nuke-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "nuke-04.ogg", 0));
  sample_pool[SND_COLLECT_NUKE] = s;


  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "power-up-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "power-up-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "power-up-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "power-up-04.ogg", 0));
  s->samples.append (new CSample (dir_snd + "power-up-05.ogg", 0));

  sample_pool[SND_POWERUP] = s;

  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "collect_animal-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "collect_animal-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "collect_animal-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "collect_animal-04.ogg", 0));
  s->samples.append (new CSample (dir_snd + "collect_animal-05.ogg", 0));
  s->samples.append (new CSample (dir_snd + "collect_animal-06.ogg", 0));
  s->samples.append (new CSample (dir_snd + "collect_animal-07.ogg", 0));
  sample_pool[SND_COLLECT_ANIMAL] = s;


  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "kill-vivi-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-vivi-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-vivi-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-vivi-04.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-vivi-05.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-vivi-06.ogg", 0));
  sample_pool[SND_VIVI_COLLISION] = s;

  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "kill-enemy-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-enemy-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-enemy-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-enemy-04.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-enemy-05.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-enemy-06.ogg", 0));
  sample_pool[SND_ENEMY01_COLLISION] = s;

  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "kill-enemy-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-enemy-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-enemy-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-enemy-04.ogg", 0));
  sample_pool[SND_ROBOTANK_COLLISION] = s;


  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "kill-labboss-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-labboss-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-labboss-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-labboss-04.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-labboss-05.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-labboss-06.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-labboss-07.ogg", 0));
  sample_pool[SND_LABBOSS_COLLISION] = s;


  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "gameover-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-04.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-05.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-06.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-07.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-08.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-09.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-10.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-11.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-12.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-13.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-14.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-15.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-16.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-17.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-18.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-19.ogg", 0));
  s->samples.append (new CSample (dir_snd + "gameover-20.ogg", 0));
  sample_pool[SND_GAMEOVER] = s;


  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "shot-enemy-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-enemy-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-enemy-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-enemy-04.ogg", 0));
  sample_pool[SND_ENEMY01_SHOOT] = s;


  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "tboss-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "tboss-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "tboss-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "tboss-04.ogg", 0));
  s->samples.append (new CSample (dir_snd + "tboss-05.ogg", 0));
  s->samples.append (new CSample (dir_snd + "tboss-06.ogg", 0));
  sample_pool[SND_TBOSS_COLLISION] = s;


  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "kill-demon-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-demon-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-demon-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-demon-04.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-demon-05.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-demon-06.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-demon-07.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-demon-08.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-demon-09.ogg", 0));
  sample_pool[SND_DEMON_COLLISION] = s;

  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "kill-roboboss-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-04.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-05.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-06.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-07.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-08.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-09.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-10.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-11.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-12.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-13.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-14.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-15.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-16.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-17.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-18.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-19.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-20.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-21.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-22.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-23.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-24.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-25.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-26.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-27.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-28.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-29.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-roboboss-30.ogg", 0));

  sample_pool[SND_ROBOBOSS_COLLISION] = s;


////////
  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "kill-zombie-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-zombie-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-zombie-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "kill-zombie-04.ogg", 0));
  sample_pool[SND_ZOMBIE_COLLISION] = s;

  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "shot-wrench-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-wrench-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-wrench-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-wrench-04.ogg", 0));
  sample_pool[SND_WRENCH_SHOT] = s;

 /* s->samples.append (new CSample (dir_snd + "shot-knife-04.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-knife-05.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-knife-06.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-knife-07.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-knife-08.ogg", 0));*/
  sample_pool[SND_KNIFE_SHOOT] = s;


  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "shot-knife-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-knife-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-knife-03.ogg", 0));
 /* s->samples.append (new CSample (dir_snd + "shot-knife-04.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-knife-05.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-knife-06.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-knife-07.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-knife-08.ogg", 0));*/
  sample_pool[SND_KNIFE_SHOOT] = s;

  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "tboss-laser-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "tboss-laser-02.ogg", 0));
  sample_pool[SND_TBOSS_SHOOT] = s;

  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "shot-demon-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-demon-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "shot-demon-03.ogg", 0));
  sample_pool[SND_DEMON_SHOOT] = s;


  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "y-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "y-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "y-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "y-04.ogg", 0));
  s->samples.append (new CSample (dir_snd + "y-05.ogg", 0));
  s->samples.append (new CSample (dir_snd + "y-06.ogg", 0));
  s->samples.append (new CSample (dir_snd + "y-07.ogg", 0));
  s->samples.append (new CSample (dir_snd + "y-08.ogg", 0));
  s->samples.append (new CSample (dir_snd + "y-09.ogg", 0));
  s->samples.append (new CSample (dir_snd + "y-10.ogg", 0));
  s->samples.append (new CSample (dir_snd + "y-11.ogg", 0));
  s->samples.append (new CSample (dir_snd + "y-12.ogg", 0));
  s->samples.append (new CSample (dir_snd + "y-13.ogg", 0));
  s->samples.append (new CSample (dir_snd + "y-14.ogg", 0));
  s->samples.append (new CSample (dir_snd + "y-15.ogg", 0));
  sample_pool[SND_Y_COLLISION] = s;


  s = new CSampleList;
  s->samples.append (new CSample (dir_snd + "explode-ufo-01.ogg", 0));
  s->samples.append (new CSample (dir_snd + "explode-ufo-02.ogg", 0));
  s->samples.append (new CSample (dir_snd + "explode-ufo-03.ogg", 0));
  s->samples.append (new CSample (dir_snd + "explode-ufo-04.ogg", 0));
  s->samples.append (new CSample (dir_snd + "explode-ufo-05.ogg", 0));
  s->samples.append (new CSample (dir_snd + "explode-ufo-06.ogg", 0));
  s->samples.append (new CSample (dir_snd + "explode-ufo-07.ogg", 0));
  sample_pool[SND_UFO_COLLISION] = s;

}
