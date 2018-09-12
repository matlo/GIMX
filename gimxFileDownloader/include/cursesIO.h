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
    //Only to be used by menuLoop
    virtual void menuHighlight() = 0;
public:
    //User must interact with this
    virtual int menuLoop(int startChoice=1) = 0;
};

class Menu : virtual public MenuBase
{
private:
    //Only to be used by menuHighlight()
    bool drawBorder;
    int bordersWE, bordersNS;

    WINDOW* menuWin;
    std::string title;
    int height, width, starty, startx, paddingy, paddingx;

    std::vector<std::string> choices;
    int numChoices, page, pageSize, highlight;

    virtual void menuHighlight();

    //Only to be used by menuLoop
	enum seekOption { back, next };
    void calculatePage(seekOption seek);
    void draw();
public:
    Menu(WINDOW* menu_win, std::vector<std::string> choices, std::string title, int pady=2, int padx=2);
    Menu(WINDOW* menu_win, int height, int width, int starty, int startx, std::vector<std::string> choices, std::string title, int pady, int padx);

    virtual int menuLoop(int startChoice=1);    

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