/*
 * display.c
 *
 *  Created on: 19 avr. 2012
 *      Author: matlo
 */

#include <string.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#include <curses.h>
#else
#include <ncurses.h>
#endif

#include "display.h"

#define CROSS_CHAR '*'

#define STICK_Y_L 11
#define STICK_X_L 21

//#define BUTTON_Y_L 3
//#define BUTTON_X_L 3

#define BUTTON_Y_L STICK_Y_L
#define BUTTON_X_L 17

#define LSTICK_Y_P 2
#define LSTICK_X_P 2

#define RSTICK_Y_P LSTICK_Y_P
#define RSTICK_X_P LSTICK_X_P + STICK_X_L + 1

#define BUTTON_Y_P LSTICK_Y_P
#define BUTTON_X_P RSTICK_X_P + STICK_X_L + 1

//#define BUTTON_Y_P LSTICK_Y_P + STICK_Y_L
//#define BUTTON_X_P LSTICK_X_P

#define BUTTON_W LABEL_LENGTH + BUTTON_X_L + 2

#define LABEL_LENGTH sizeof("triangle")
#define BUTTON_LENGTH LABEL_LENGTH + sizeof(" - 255") + 1

char* buts[BUTTON_NB] =
{
  "select",
  "start",
  "ps",
  "up",
  "right",
  "down",
  "left",
  "triangle",
  "circle",
  "cross",
  "square",
  "l1",
  "r1",
  "l2",
  "r2",
  "l3",
  "r3",
};

WINDOW *lstick, *rstick, *wbuttons;

int cross[2][2] = { {STICK_X_L / 2, STICK_Y_L / 2}, {STICK_X_L / 2, STICK_Y_L / 2} };

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

  refresh();

  lstick = newwin(STICK_Y_L, STICK_X_L, LSTICK_Y_P, LSTICK_X_P);
  box(lstick, 0 , 0);
  wrefresh(lstick);

  mvprintw(1, LSTICK_X_P + 1, "Left stick");
  refresh();

  rstick = newwin(STICK_Y_L, STICK_X_L, RSTICK_Y_P, RSTICK_X_P);
  box(rstick, 0 , 0);
  wrefresh(rstick);

  mvprintw(1, RSTICK_X_P + 1, "Right stick");
  refresh();

  wbuttons = newwin(BUTTON_Y_L, BUTTON_X_L, BUTTON_Y_P, BUTTON_X_P);
  box(wbuttons, 0 , 0);
  wrefresh(wbuttons);

  mvprintw(1, BUTTON_X_P + 1, "Buttons");
  refresh();
}

void display_end()
{
  endwin();
}

int last_button_nb = 0;

void display_run(int axes[4], int max_axis, int buttons[BUTTON_NB], int max_button)
{
  int i;
  int d;
  char label[LABEL_LENGTH + 7];

  d = 0;

  for(i=0; i<BUTTON_NB; ++i)
  {
    if(buttons[i])
    {
      snprintf(label, sizeof(label), "%8s - %d", buts[i], buttons[i]);
      mvwaddstr(wbuttons, 1 + d, 1, label);
      d++;
    }
    wnoutrefresh(wbuttons);
    if(d == BUTTON_Y_L - 3)
    {
      break;
    }
  }
  snprintf(label, sizeof(label), "%22s", "");
  for(i=d; i<last_button_nb; ++i)
  {
    mvwaddstr(wbuttons, 1 + i, 1, label);
  }
  last_button_nb = d;

  mvwaddch(lstick, cross[0][1], cross[0][0], ' ');
  cross[0][0] = STICK_X_L / 2 + (double)axes[0] / max_axis * (STICK_X_L / 2 - 1);
  cross[0][1] = STICK_Y_L / 2 + (double)axes[1] / max_axis * (STICK_Y_L / 2 - 1);
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
  cross[1][0] = STICK_X_L / 2 + (double)axes[2] / max_axis * (STICK_X_L / 2 - 1);
  cross[1][1] = STICK_Y_L / 2 + (double)axes[3] / max_axis * (STICK_Y_L / 2 - 1);
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
