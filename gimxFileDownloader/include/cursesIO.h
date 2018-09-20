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
    std::string fillString(int textLength, char filler=' ');
    void clrToEolFrom(WINDOW* win, int y, int x);
        //Overwrites with " "
    void eraseChunk(WINDOW* win, int y, int x, int amount);
    void eraseChunk(WINDOW* win, int startLine, int endLine, int startX, int endX, int amount);
    int maxLines(int height, int padding=0);
    int maxChars(int screenWidth, int padding=0);

    class ProgressBar
    {
        private:
            char barChar[2];
            char point[2];
            std::string prefix;
            std::string suffix;
            int barSize;
            int prevAmount;

            WINDOW* window;
            int startY, startX, currentX;

            /*TODO create init() which fills in data like constructor. Also create clear().
            Expose thin wrapper of init(), called reset(), which first calls clear().
            The aim of these additions is to make this class re-usable*/
            void init(WINDOW* win, int size, int startY, int startX, std::string prefix="[", char barChar='=', char point='>',
             std::string suffix="]");

            void parseFormat(char barChar, char point);

        public:
            ProgressBar(WINDOW* win, int size, int startY, int startX, std::string prefix="[", char barChar='=', char point='>',
              std::string suffix="]");
            void reset();
            void reset(WINDOW* win, int size, int startY, int startX, std::string prefix="[", char barChar='=', char point='>',
             std::string suffix="]");

            void first();
            void update(double progress);

            void setWindow(WINDOW* newWin) { window = newWin; }
            void setSize(int size) { barSize = size - (prefix.length() + suffix.length()); }
            void setStartCoordinates(int y, int x) { startY = y; startX = x; currentX = x; }
    };


    struct WinData
    {
        /*WinData(WINDOW* menu_win, std::string title, int height, int width, int startY, int startX, int padY, int padX, \
          bool drawBorder, int bordersWE, int bordersNS);*/
        bool drawBorder;
        int bordersWE, bordersNS;

        WINDOW* win;
        int height, width, startY, startX, paddingY, paddingX;
    };
    //Recommended to store raw pointer in smart pointer
    WinData* newWinData(WINDOW* window, int height=0, int width=0, int startY=0, int startX=0, int padY=2, int padX=2, \
      bool drawBorder=true, int bordersWE=0, int bordersNS=0);
      

    class Menus
    {
    protected:
        WinData* winData;
        std::string title;

        //Common math
        virtual int _maxLines(int padY=0) { return maxLines(winData->height, padY == 0 ? winData->paddingY : padY); }
        virtual int _maxChars(int padX=0) { return maxChars(winData->width, padX == 0 ? winData->paddingX : padX); }
    public:
        Menus(WinData* windowsData, std::string title) : winData(windowsData), title(title) { }

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
        SelectionMenu(WinData* windowsData, std::vector<std::string> choices, std::string title="Please chosen an option");

        void menuLoop(int startChoice=1);
        void getResult(std::vector<int>& chosen);

        void setDrawBorder(bool draw=true, int bordersWE=0, int bordersNS=0);
    };

    class ttyProgressDialog : public Menus
    {
    private:
        std::string message;

        int progInfoPosY, progInfoPosX;
        int pBarPosY, pBarPosX;

        std::string progInfo;
        ProgressBar pBar;

    public:
        ttyProgressDialog(WinData* windowsData, std::string title="Progress", std::string mssg="");

        void setDrawBorder(bool draw=true, int bordersWE=0, int bordersNS=0);

        void dialog();
        bool update(double progress, std::string mssg="");
        void resetPBar() { pBar.reset(); }
    };

}

#endif //CURSESIO_H