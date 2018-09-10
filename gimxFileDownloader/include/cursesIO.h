/*
 * ncursesIO.h
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 10 Aug. 2018
 */

#ifndef  CURSESIO_H
#define  CURSESIO_H

#include <vector>
#include <string>

#ifdef WIN32
#include <cursesw.h>
#else
#include <ncursesw/ncurses.h>
#endif





class MenuBase
{
protected:
    WINDOW* menuWin;
    int height, width, starty, startx;

    std::vector<std::string> choices;
    int numChoices;

    //Only to be used by menuLoop
    virtual void menuHighlight(WINDOW *menu_win, int highlight, int xStart=2, int yStart=2) = 0;
public:
    MenuBase(WINDOW* menu_win, std::vector<std::string> choices) : MenuBase(menu_win, LINES, COLS, 0, 0, choices) { }
    MenuBase(WINDOW* menu_win, int height, int width, int starty, int startx, std::vector<std::string> choices);

    //User must interact with this
    virtual int menuLoop(int startChoice=1) = 0;
    
    //Getters and setters allow for customisation
    //Setters
    virtual void setKeypad(bool state=true) { keypad(this->menuWin, state); }
};

class Menu : virtual public MenuBase
{
private:
    //Only to be used by menuHighlight()
    void pageUp();
    void pageDown();
    bool drawBorder;
    int bordersWE, bordersNS;

    virtual void menuHighlight(WINDOW *menu_win, int highlight, int xStart=2, int yStart=2);
public:
    Menu(WINDOW* menu_win, std::vector<std::string> choices) \
        : Menu(menu_win, LINES, COLS, 0, 0, choices) { }
    Menu(WINDOW* menu_win, int height, int width, int starty, int startx, std::vector<std::string> choices);

    virtual int menuLoop(int startChoice=1);

    //Setters
    void setPrintOrientation(bool orientation);
    void setDrawBorder(bool draw=true, int bordersWE=0, int bordersNS=0);
};

class ttyProgressDialog
{
public:
    ttyProgressDialog(WINDOW* dialog_win, int height, int width) : ttyProgressDialog(dialog_win, "Downloading", height, width, 0, 0, "Progress") { }
    ttyProgressDialog(WINDOW* dialog_win, std::string title, int height, int width, int starty, int startx, std::string message);

    void setDrawBorder(bool draw=true, int bordersWE=0, int bordersNS=0);

    void dialog();
    bool Update(double progress, std::string message);

private:
    WINDOW* dialogWin;
    std::string title;
    int height, width, starty, startx;
    bool drawBorder;
    int bordersWE, bordersNS;

    std::string message;
};


#endif //CURSESIO_H