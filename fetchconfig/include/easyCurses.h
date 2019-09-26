/*
 * easyCurses.h
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 10 Aug. 2018
 */

#ifndef  EASYCURSES_H
#define  EASYCURSES_H

#include <vector>
#include <map>
#include <array>
#include <string>
#include <functional> //std::function

#ifdef WIN32
#include <cursesw.h>
#else
#include <ncursesw/ncurses.h>
#endif

namespace EasyCurses
{
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Miscellaneous

    unsigned roundUp(float n);

    unsigned centreText(unsigned windowWidth, unsigned textLength);

    std::string fillString(unsigned textLength, char filler=' ');

    void clrToEolFrom(WINDOW* win, unsigned y, unsigned x);

        //Overwrites with " "
    void eraseChunk(WINDOW* win, unsigned y, unsigned x, unsigned amount);
    void eraseChunk(WINDOW* win, unsigned startY, unsigned endY, unsigned startX, unsigned endX);

    unsigned maxLines(unsigned height, unsigned padding=0);

    unsigned maxChars(unsigned screenWidth, unsigned padding=0);

    namespace TextFormat
    {
        /*
         * Example of text handling:
         *
         * Original text:
         *   "do\nsome really long text\nwork"
         *
         * Formatting information:
         *
         *   page = 0
                     00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
         *   text = "d  o  \n s  o  m  e  _  r  e  a  l  l  y  _  l  o  n  g  _  t  e  x  t  \n w  o  r  k"
         *                                      <---------screen width-------->
         *    <---------screen width-------->   <---------------overflow------------>
         *     00|01|02|03|04|05|06|07|08|09|   |00|01|02|03|04|05|06|07|08|09|10|11|12|
         *   0| d| o|\n|  |  |  |  |  |  |  |   |  |  |  |  |  |  |  |  |  |  |  |  |  |
         *   1| s| o| m| e|  | r| e| a| l| l|   | y|  | l| o| n| g|  | t| e| x| t|\n|  |
         *   2| w| o| r| k|  |  |  |  |  |  |   |  |  |  |  |  |  |  |  |  |  |  |  |  |
         *
         *
         *                         { [index] = (end point, # characters) };
         *   LineEnds pageLayout = { [0] = (2, 2), [1] = (13, 11), [2] = (29, 4) };
         *     => If a line overflows, it has a special attribute.
         *        Its number of characters = the screen width +1. Simply subtract -1 to
         *        get the real end point.
         *
         *                       { (line #, (end point, # characters)) }
         *   OverFlow oFLayout = { [1] = (23, 10), [1] = (24, 1) };
         *     => 'oFLayout' contains the virtual end points in between,
         *        and the end point of the new line of the overflow.
         *
         *
         * Must be recalculated at every screen resize.
         */

//                         index of line end, # characters
        typedef std::vector<std::pair<size_t, size_t>> LineEnds;
 //                           line #, index of line end, # characters
        typedef std::multimap<size_t, std::pair<size_t, unsigned>> OverFlow;

        void overflow(std::string text, unsigned maxLength, LineEnds& lineFormat,
          OverFlow& oFLayout, bool wrap=false);
        void overflow(std::string text, unsigned maxLength, LineEnds& lineFormat);
        /*
         * Return 1 if on the first line, and the 'endPoint' is negative.
         * Return 2 if both 'endPoint' and 'prevEndPoint' are negative.
         * Return 3 if 'endPoint' is negative and 'prevEndPoint' is positive.
         * Return 4 if 'endPoint' is positive, but 'prevEndPoint' is negative.
         * Return 0 if both 'endPoint' and 'prevEndPoint' are positive.
         */
    }
    namespace TF = TextFormat;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Widgets

    class ProgressBar
    {
        private:
            char barChar[2];
            char point[2];
            std::string prefix;
            std::string suffix;
            unsigned barSize;
            unsigned prevAmount;

            WINDOW* window;
            unsigned startY, startX, currentX;

            void parseFormat(char barChar, char point);

        public:
            ProgressBar(WINDOW* win, unsigned size, unsigned startY, unsigned startX, std::string prefix="[",
              std::string suffix="]", char barChar='=', char point='>');
            void reset();

            void first();
            void update(double progress);

            //Getters and setters
            void setWindow(WINDOW* newWin) { window = newWin; }
            void setSize(unsigned size) { barSize = size - (prefix.length() + suffix.length()); }
            void setStartCoords(unsigned y, unsigned x) { startY = y; startX = x; currentX = x; }
            void setPrefix(std::string p) { prefix     = p; }
            void setSuffix(std::string s) { suffix     = s; }
            void setBarChar(char c)       { barChar[0] = c; }
            void setPointChar(char c)     { point[0]   = c ;}
    };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Menus

    struct WinData
    {
        bool drawBorder;
        int bordersWE, bordersNS;

        WINDOW* win;
        unsigned height, width, startY, startX, paddingY, paddingX;
    };
    //Recommended to store raw pointer, on heap mem, in smart pointer
    WinData* newWinData(WINDOW* window, unsigned height=0, unsigned width=0, unsigned startY=0, unsigned startX=0, unsigned padY=2, unsigned padX=2, \
      bool drawBorder=true, int bordersWE=0, int bordersNS=0);


    class Menus
    {
    protected:
        WinData* winData;
        std::string title;

        //Common math
        virtual unsigned _maxLines(unsigned padY=0) { return maxLines(winData->height, padY == 0 ? winData->paddingY : padY); }
        virtual unsigned _maxChars(unsigned padX=0) { return maxChars(winData->width, padX == 0 ? winData->paddingX : padX); }

        virtual void drawTitle();
        virtual void drawFrame();

    public:
        Menus(WinData* windowsData, std::string title) : winData(windowsData), title(title) { }
        virtual ~Menus() { }

        //TODO implement capability to re-use menus
        void setTitle(std::string title) { this->title = title; }
        void setWinData(WinData* wDat) { winData = wDat; }
    };


    enum class NavContent
    {
        null, left, right, lineUp, lineDown, pageUp, pageDown, select,
        finish, custom
    };

    class BasicMenu : public Menus
    {
    protected:
        //Input
        std::map<unsigned, NavContent> keyBindings;
        typedef std::function<bool(void)> F;
        F cusAct;
        unsigned getInput();
        NavContent mapInput(unsigned rawInput);
        virtual void inputHandling(NavContent& input);

        //Text formatting
        unsigned page, numLines;
        std::string text;
        TF::LineEnds pageLayout;
        TF::OverFlow oFLayout;

            //Common math
        unsigned pageTop()    { return page * _maxLines(); }
        unsigned pageBottom()
        {
            if(page == lastPage() -1)
                return numLines -1;
            return ( (page +1) * (_maxLines()) ) -1;
        } //page is zero-initialised, so +1.
                                                                    //-1 as result is a line number - which is zero-initialised.
        unsigned lastPage()   { return roundUp( float(numLines) / float(_maxLines()) ); }

            //Text painting
        virtual void calculatePage(NavContent seek);
        virtual void printStyle(); //Only to be used by drawContent
        virtual void drawContent();
        virtual void drawPageNumber(); //Only to be used by update and drawFrame


        //Update
        bool changed = false;
        void setUpdate() { changed = true; }
        bool doUpdate();
        virtual void update();

    public:
        BasicMenu(std::string lines, WinData* windowsData, std::string title="Please chosen an option");
        virtual ~BasicMenu() { }

        virtual void menuLoop();

        virtual void setDrawBorder(bool draw=true, int bordersWE=0, int bordersNS=0);

        //Getters and setters
        void setKeyBindings(std::map<unsigned, NavContent>& kB)
          { keyBindings = kB; }
        std::map<unsigned, NavContent>& getKeyBindings()
          { return keyBindings; }
        void setCustomAction(F cusAct) { this->cusAct = cusAct; }
        F& getCustomAction() { return cusAct; }
        void setText(std::string text) { this->text = text; }
    };


    class SelectionMenu : public BasicMenu
    {
    private:
        //Input
        std::map<size_t, bool> selected;
        virtual void inputHandling(NavContent& input) override;

        //Text formatting
        std::string checkMark = "X";
        std::string blankMark = "O";
        size_t highlight;
        unsigned oFLine;

            //Common math
        unsigned yCoord() { return (highlight + winData->paddingY) - (_maxLines() * page); }

            //Text painting
        virtual void calculatePage(NavContent seek) override;
        virtual void printStyle() override; //Only to be used by drawContent
        virtual void drawFrame() override;
        void drawCheckMark(unsigned index, unsigned y);
        void drawAllCheckMarks();
        void navTrunc(NavContent input);

        //Update
        virtual void update() override;
        void updateLineTrackers(unsigned n);

    public:
        SelectionMenu(std::string text, WinData* windowsData, std::string title="Please chosen an option");

        void menuLoop(unsigned startChoice=0);
        void getResult(std::vector<unsigned>& chosen);
    };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Dialogs

    class ttyProgressDialog : public Menus
    {
    private:
        ProgressBar pBar;
        unsigned progInfoPosY, progInfoPosX;
        unsigned pBarPosY, pBarPosX;

        std::string message;

        std::string progInfo;

    public:
        ttyProgressDialog(WinData* windowsData, std::string title="Progress", std::string mssg="");

        void setDrawBorder(bool draw=true, int bordersWE=0, int bordersNS=0);

        void dialog();
        bool update(double progress, std::string mssg="");
        void resetPBar() { pBar.reset(); }
    };

}

#endif //EASYCURSES_H
