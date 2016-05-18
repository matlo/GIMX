/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include <string.h>
#include <stdlib.h>
#ifdef WIN32
#include <cursesw.h>
#else
#include <ncursesw/ncurses.h>
#endif

#include "display.h"

#include "calibration.h"
#include <ginput.h>
#include "gimx.h"
#include <adapter.h>
#include <stats.h>

#define CROSS_CHAR '*'
#define SHIFT_ESC _("Press Shift+Esc to exit.")

#ifndef WIN32
#define STICK_Y_L 11
#else
#define STICK_Y_L 13
#endif
#define STICK_X_L 21

//#define BUTTON_Y_L 3
//#define BUTTON_X_L 3

#define BUTTON_Y_L STICK_Y_L
#define BUTTON_X_L 16

#define LSTICK_Y_P 2
#define LSTICK_X_P 2

#define RSTICK_Y_P LSTICK_Y_P
#define RSTICK_X_P LSTICK_X_P + STICK_X_L + 1

#define BUTTON_Y_P LSTICK_Y_P
#define BUTTON_X_P RSTICK_X_P + STICK_X_L + 1

#define CAL_Y_P LSTICK_Y_P + STICK_Y_L
#define CAL_X_P 2

//#define BUTTON_Y_P LSTICK_Y_P + STICK_Y_L
//#define BUTTON_X_P LSTICK_X_P

#define BUTTON_W LABEL_LENGTH + BUTTON_X_L + 2

#define LABEL_LENGTH sizeof("triangle")
#define BUTTON_LENGTH LABEL_LENGTH + sizeof(": 255")

static WINDOW *lstick = NULL, *rstick = NULL, *wbuttons = NULL, *wcal = NULL;

static int cross[2][2] = { {STICK_X_L / 2, STICK_Y_L / 2}, {STICK_X_L / 2, STICK_Y_L / 2} };

void display_calibration()
{
  char line[COLS];
  s_mouse_cal* mcal = cal_get_mouse(current_mouse, current_conf);

  if(current_cal == NONE)
  {
    mvaddstr(CAL_Y_P, CAL_X_P + 1, _("Mouse calibration (Ctrl+F1 to edit)"));
  }
  else
  {
    mvaddstr(CAL_Y_P, CAL_X_P + 1, _("Mouse calibration (Ctrl+F1 to save)(mouse wheel to change values)"));
  }
  clrtoeol();
  wmove(wcal, 1, 1);
  if(ginput_get_mk_mode() == GE_MK_MODE_MULTIPLE_INPUTS)
  {
    waddstr(wcal, "Mouse:");
    if(current_cal == MC)
    {
      wattron(wcal, COLOR_PAIR(4));
    }
    snprintf(line, COLS, " %s (%d) (F1) ", ginput_mouse_name(current_mouse), ginput_mouse_virtual_id(current_mouse));
    waddstr(wcal, line);
    if(current_cal == MC)
    {
      wattron(wcal, COLOR_PAIR(1));
    }
  }
  waddstr(wcal, _("Profile:"));
  if(current_cal == CC)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  snprintf(line, COLS, " %d (F2)", current_conf + 1);
  waddstr(wcal, line);
  if(current_cal == CC)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  mvwaddstr(wcal, 2, 1, _("Dead zone:"));
  if(current_cal == DZX)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, " x=");
  if(mcal->dzx)
  {
    snprintf(line, COLS, "%d", *mcal->dzx);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F3)");
  if(current_cal == DZX)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(current_cal == DZY)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, " y=");
  if(mcal->dzy)
  {
    snprintf(line, COLS, "%d", *mcal->dzy);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F4)");
  if(current_cal == DZY)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(current_cal == DZS)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, _(" shape="));
  if(mcal->dzs)
  {
    if (*mcal->dzs == E_SHAPE_CIRCLE)
    {
      waddstr(wcal, _("circle"));
    }
    else
    {
      waddstr(wcal, _("rectangle"));
    }
  }
  else
  {
    waddstr(wcal, _(" N/A"));
  }
  waddstr(wcal, " (F5)");
  if(current_cal == DZS)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  mvwaddstr(wcal, 3, 1, _("Acceleration:"));
  if(current_cal == TEST)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, _(" test (F6)"));
  if(current_cal == TEST)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(current_cal == EX)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, " x=");
  if(mcal->ex)
  {
    snprintf(line, COLS, "%.2f", *mcal->ex);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F7)");
  if(current_cal == EX)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(current_cal == EY)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, " y=");
  if(mcal->ey)
  {
    snprintf(line, COLS, "%.2f", *mcal->ey);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F8)");
  if(current_cal == EY)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  mvwaddstr(wcal, 4, 1, _("Sensitivity:"));
  if(current_cal == MX)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  if(mcal->mx)
  {
    snprintf(line, COLS, " %.2f", *mcal->mx);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _(" N/A"));
  }
  waddstr(wcal, " (F9)");
  if(current_cal == MX)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  mvwaddstr(wcal, 5, 1, "X/Y:");
  if(current_cal == RD || current_cal == VEL)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, _(" circle test"));
  if(current_cal == RD || current_cal == VEL)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  waddstr(wcal, ", ");
  if(current_cal == RD)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  snprintf(line, COLS, _("radius=%d (F10)"), mcal->rd);
  waddstr(wcal, line);
  if(current_cal == RD)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  waddstr(wcal, ", ");
  if(current_cal == VEL)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  snprintf(line, COLS, _("velocity=%d (F11)"), mcal->vel);
  waddstr(wcal, line);
  if(current_cal == VEL)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  if(current_cal == MY)
  {
    wattron(wcal, COLOR_PAIR(4));
  }
  waddstr(wcal, _(" ratio="));
  if(mcal->mx && mcal->my)
  {
    snprintf(line, COLS, "%.2f", *mcal->my / *mcal->mx);
    waddstr(wcal, line);
  }
  else
  {
    waddstr(wcal, _("N/A"));
  }
  waddstr(wcal, " (F12)");
  if(current_cal == MY)
  {
    wattron(wcal, COLOR_PAIR(1));
  }
  wclrtoeol(wcal);
  box(wcal, 0 , 0);
  wnoutrefresh(wcal);
  doupdate();
}

void display_init()
{
  initscr();

  if(has_colors() == FALSE)
  {
    endwin();
    printf("Your terminal does not support color\n");
    exit(1);
  }
  start_color();
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_WHITE, COLOR_WHITE);
  init_pair(3, COLOR_BLACK, COLOR_RED);
  init_pair(4, COLOR_RED, COLOR_BLACK);

  wrefresh(stdscr);//first call clears the screen
  
  mvaddstr(1, LSTICK_X_P + 1, _("Left stick"));
  mvaddstr(1, RSTICK_X_P + 1, _("Right stick"));
  mvaddstr(1, BUTTON_X_P + 1, _("Buttons"));

  lstick = newwin(STICK_Y_L, STICK_X_L, LSTICK_Y_P, LSTICK_X_P);
  box(lstick, 0 , 0);
  wnoutrefresh(lstick);

  rstick = newwin(STICK_Y_L, STICK_X_L, RSTICK_Y_P, RSTICK_X_P);
  box(rstick, 0 , 0);
  wnoutrefresh(rstick);

  wbuttons = newwin(BUTTON_Y_L, BUTTON_X_L, BUTTON_Y_P, BUTTON_X_P);
  box(wbuttons, 0 , 0);
  wnoutrefresh(wbuttons);

  mvaddstr(CAL_Y_P, CAL_X_P + 1, _("Mouse calibration (Ctrl+F1 to edit)"));

  wcal = newwin(7, COLS-3, CAL_Y_P + 1, CAL_X_P);
  box(wcal, 0 , 0);
  wnoutrefresh(wcal);

  mvaddstr(LINES-1, 1, _("Refresh rate:"));
  mvaddstr(LINES-1, COLS-strlen(SHIFT_ESC), SHIFT_ESC);

  doupdate();
}

void display_end()
{
  if(wcal)
  {
    endwin();
  }
}

static int last_button_nb = 0;

void display_run(e_controller_type type, int axis[])
{
  int i;
  int d;
  char label[BUTTON_LENGTH];
  char rate[COLS];

  int freq = stats_get_frequency(0);

  if(freq >= 0)
  {
    sprintf(rate, _("Refresh rate: %4dHz  "), freq);
    mvaddstr(LINES-1, 1, rate);
  }

  d = 0;

  for(i=rel_axis_rstick_y+1; i<AXIS_MAX; ++i)
  {
    if(axis[i])
    {
      snprintf(label, sizeof(label), "%8s: %4d", controller_get_axis_name(type, i), axis[i]);
      mvwaddstr(wbuttons, 1 + d, 1, label);
      d++;
      if(d == BUTTON_Y_L - 3)
      {
        break;
      }
    }
  }
  memset(label, ' ', sizeof(label));
  label[sizeof(label)-1] = '\0';
  for(i=d; i<last_button_nb; ++i)
  {
    mvwaddstr(wbuttons, 1 + i, 1, label);
  }
  last_button_nb = d;
  wnoutrefresh(wbuttons);

  mvwaddch(lstick, cross[0][1], cross[0][0], ' ');
  cross[0][0] = STICK_X_L / 2 + (double)axis[rel_axis_lstick_x] / controller_get_max_signed(adapter_get(0)->ctype, rel_axis_lstick_x) * (STICK_X_L / 2 - 1);
  cross[0][1] = STICK_Y_L / 2 + (double)axis[rel_axis_lstick_y] / controller_get_max_signed(adapter_get(0)->ctype, rel_axis_lstick_y) * (STICK_Y_L / 2 - 1);
  if(cross[0][0] <= 0 || cross[0][0] >= STICK_X_L-1 || cross[0][1] <= 0 || cross[0][1] >= STICK_Y_L-1)
  {
    mvwaddch(lstick, cross[0][1], cross[0][0], CROSS_CHAR | COLOR_PAIR(3));
  }
  else
  {
    mvwaddch(lstick, cross[0][1], cross[0][0], CROSS_CHAR);
  }
  wnoutrefresh(lstick);

  mvwaddch(rstick, cross[1][1], cross[1][0], ' ');
  cross[1][0] = STICK_X_L / 2 + (double)axis[rel_axis_rstick_x] / controller_get_max_signed(adapter_get(0)->ctype, rel_axis_rstick_x) * (STICK_X_L / 2 - 1);
  cross[1][1] = STICK_Y_L / 2 + (double)axis[rel_axis_rstick_y] / controller_get_max_signed(adapter_get(0)->ctype, rel_axis_rstick_y) * (STICK_Y_L / 2 - 1);
  if(cross[1][0] <= 0 || cross[1][0] >= STICK_X_L-1 || cross[1][1] <= 0 || cross[1][1] >= STICK_Y_L-1)
  {
    mvwaddch(rstick, cross[1][1], cross[1][0], CROSS_CHAR | COLOR_PAIR(3));
  }
  else
  {
    mvwaddch(rstick, cross[1][1], cross[1][0], CROSS_CHAR);
  }
  wnoutrefresh(rstick);

  move(LINES-1, COLS-1);
  wnoutrefresh(stdscr);
  doupdate();
}

/*int main(int argc, char* argv[])
{
  int i;
  display_init();
  int axes[4] = {16,16,0,0};
  int buttons[BUTTON_NB] = {1, 0, 1};

  for(i=0; i<10000; ++i)
  {
    display_run(axes, 32, buttons, 1);
    usleep(10000);
  }

  display_end();

  return 0;
}*/
