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
#include <map>
#include <string>
#include <math.h> //ceil

#ifdef WIN32
#include <cursesw.h>
#else
#include <ncursesw/ncurses.h>
#endif

namespace EasyCurses
{
    //Miscellaneous
    int centreText(int screenWidth, int textLength, int padding=0);
    std::string fillString(int textLength, std::string filler=" ");

    int maxLines(int height, int padding=0);
    int maxChars(int screenWidth, int padding=0);


    struct WinData
    {
        /*WinData(WINDOW* menu_win, std::string title, int height, int width, int startY, int startX, int padY, int padX, \
          bool drawBorder, int bordersWE, int bordersNS);*/
        bool drawBorder;
        int bordersWE, bordersNS;

        WINDOW* menuWin;
        std::string title;
        int height, width, startY, startX, paddingY, paddingX;
    };
    //Recommended to store raw pointer in smart pointer
    WinData* newWinData(WINDOW* menu_win, std::string title="", int height=0, int width=0, int startY=0, int startX=0, int padY=2, int padX=2, \
      bool drawBorder=true, int bordersWE=0, int bordersNS=0);
      

    class Menus
    {
    protected:
        WinData* winData;

        //Common math
        virtual int _maxLines() { return maxLines(winData->height, winData->paddingY); }
        virtual int _maxChars() { return maxChars(winData->width, winData->paddingX); }
    public:
        Menus(WinData* windowsData) : winData(windowsData) { }

        //TODO implement capability to re-use menus
        //virtual void reset() = 0;
    };

    class SelectionMenu : public Menus
    {
    private:
        std::vector<std::string> choices;
        std::map<int,std::string> selected;

        std::multimap<int,std::string> truncated;
        //Stores info about what piece of truncated string to show
        //        First set of strings matching trunc line , second set of strings matching trunc line
        std::pair<std::multimap<int,std::string>::iterator, std::multimap<int,std::string>::iterator> beginNEnd;
        std::multimap<int,std::string>::iterator it;

        int numChoices, page, highlight;

        std::string checkMark = "X";
        std::string blankMark = "O";

        //Common math
        int currentLine() { return highlight -1 +winData->paddingY - (_maxLines() * (page -1)); }
        int pageBottom() { return ((page -1) * _maxLines()) +1; }
        int lastPage() { return ceil( float(numChoices) / float(_maxLines()) ); }
        
        virtual void menuHighlight();

        //Only to be used by menuLoop
        enum seekOption { back, next };
        void calculatePage(seekOption seek);
        void draw();
        void reDrawCheckMark(int index, int y);
        void reDrawAllCheckMarks();
        void prepareStr(std::string& text, int line);
        void truncNav(seekOption way, int& input);
    public:
        SelectionMenu(WinData* windowsData, std::vector<std::string> choices);

        void menuLoop(int startChoice=1);
        void getResult(std::vector<int>& chosen);

        void setDrawBorder(bool draw=true, int bordersWE=0, int bordersNS=0);
    };

    class ttyProgressDialog : public Menus
    {
    private:
        std::string message;

    public:
        ttyProgressDialog(WinData* windowsData, std::string mssg);

        void setDrawBorder(bool draw=true, int bordersWE=0, int bordersNS=0);

        void dialog();
        bool Update(double progress, std::string message);
    };

}

#endif //CURSESIO_H