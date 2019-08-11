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

    int roundUp(float n);

    int centreText(int windowWidth, int textLength);

    std::string fillString(int textLength, char filler=' ');

    void clrToEolFrom(WINDOW* win, int y, int x);

        //Overwrites with " "
    void eraseChunk(WINDOW* win, int y, int x, int amount);
    void eraseChunk(WINDOW* win, int startY, int endY, int startX, int endX);

    int maxLines(int height, int padding=0);

    int maxChars(int screenWidth, int padding=0);

    namespace TextFormat
    {
        /* Example of text handling:
         *
         * Original text:
         *   "camp\nsome really long text\nwork"
         *
         * Formatting information:
         *
         *   page = 0
         *     <-------------screen width------------>   <---------overflow-------->
         *     00|01|02|03|04|05|06|07|08|09|10|11|12|   |13|14|15|16|17|18|19|20|21|
         *   0| c| a| m| p|\n|  |  |  |  |  |  |  |  |   |  |  |  |  |  |  |  |  |  |
         *   1| s| o| m| e|  | r| e| a| l| l| y|  | l|   | o| n| g|  | t| e| x| t|\n|
         *   2| w| o| r| k|  |  |  |  |  |  |  |  |  |   |  |  |  |  |  |  |  |  |  |
         *
         *
         *   pageLayout = { [0] = 4, [0] = 18, [0] = -31  }
         *     => -31 denotes the previous line was the end of an overflowing line. Negating it
         *        denotes this whilst still storing the end of line, to preserve patterns
         *        dependent on indexes.
         *
         *   overflowLayout = { [1] = 18, [1] = 26 }
         */

        /*
         * Existing newlines must still be filtered as this function only marks the end of lines.
         * Must also be recalculated at every screen resize
         */

        //This records the position of each line's end, including those marked by virtual "/n". It must be updated with each screen resize
        typedef std::vector<int> LineEnds;
        typedef std::multimap<int, int> TextLayout;

        void overflow(std::string text, int maxLength, LineEnds& markers, bool markVirtEnd=false);
        /*
         * Return 1 if on the first line, and the 'endPoint' is negative.
         * Return 2 if both 'endPoint' and 'prevEndPoint' are negative.
         * Return 3 if 'endPoint' is negative and 'prevEndPoint' is positive.
         * Return 4 if 'endPoint' is positive, but 'prevEndPoint' is negative.
         * Return 0 if both 'endPoint' and 'prevEndPoint' are positive.
         */
        unsigned isOverflow(LineEnds markers, int index);
        void pageFormat(int maxLines, LineEnds markers, TextLayout& pageLayout, TextLayout& overflowLayout);
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
            int barSize;
            int prevAmount;

            WINDOW* window;
            int startY, startX, currentX;


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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Menus

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

        virtual void drawTitle();
        virtual void drawFrame();

    public:
        Menus(WinData* windowsData, std::string title) : winData(windowsData), title(title) { }
        virtual ~Menus() { }

        //TODO implement capability to re-use menus
        //virtual void reset() = 0;
    };


    enum class NavContent
    {
        null, left, right, lineUp, lineDown, pageUp, pageDown, select,
        finish, custom
    };

    class BasicMenu : public Menus
    {
    protected:
            /*
            * From Menus:
            *   - WinData* winData
            *   - std::string title
            */

        //Input
        typedef std::function<bool(void)> F;
        F cusAct;
        int getInput();
        NavContent mapInput(int rawInput);
        virtual void inputHandling(NavContent& input);

        //Text formatting
        int page, numLines;
        std::string text;

            //Common math
        int pageTop()    { return page * _maxLines(); }
        int pageBottom() { return ( (page +1) * (_maxLines()) ) -1; } //page is zero-initialised, so +1.
                                                                    //-1 as result is a line number - which is zero-initialised.
        int lastPage()   { return roundUp( float(numLines) / float(_maxLines()) ); }

            //Text painting
        virtual void calculatePage(NavContent seek);
        virtual void printStyle(int& x, int& y); //Only to be used by drawContent
        virtual void drawContent();
        virtual void drawPageNumber(); //Only to be used by update and drawFrame


        //Update
        bool changed = false;
        void setUpdate() { changed = true; }
        bool doUpdate();
        virtual void update();

    private:
        //Text formatting
        TF::LineEnds lineFormat;

    public:
        BasicMenu(std::string text, WinData* windowsData, std::string title="Please chosen an option");
        virtual ~BasicMenu() { }

        virtual void menuLoop();

        virtual void setDrawBorder(bool draw=true, int bordersWE=0, int bordersNS=0);

        static std::map<int, NavContent> keyBindings;
        void setCustomAction(F cusAct);
        F& getCustomAction();
    };


    class SelectionMenu : public BasicMenu
    {
    private:
        /*
         * From Menus:
         *   - WinData* winData
         *   - std::string title
         *
         * From BasicMenu:
         *   - int page, numLines
         *   - std::string text
         */

        //Input
        std::map<int,bool> selected;
        virtual void inputHandling(NavContent& input) override;

        //Text formatting
        std::string checkMark = "X";
        std::string blankMark = "O";
        TF::TextLayout::iterator overflowLine;
        //<page number, end of line in text string>
        TF::TextLayout pageLayout;
        //<line number, end of line in text string>
        TF::TextLayout overflowLayout;
        int highlight;

            //Common math
        int currentLine() { return (highlight + winData->paddingY) - (_maxLines() * page); }

            //Text painting
        virtual void calculatePage(NavContent seek) override;
        virtual void printStyle(int& x, int& y) override; //Only to be used by drawContent
        virtual void drawFrame() override;
        void drawCheckMark(int index, int y);
        void drawAllCheckMarks();
        void navTrunc(NavContent input);

        //Update
        virtual void update() override;
        void updateLineTrackers(int n) { highlight = n; overflowLine = overflowLayout.find(highlight); }

    public:
        SelectionMenu(std::string text, WinData* windowsData, std::string title="Please chosen an option");

        void menuLoop(int startChoice=0);
        void getResult(std::vector<int>& chosen);
    };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Dialogs

    class ttyProgressDialog : public Menus
    {
    private:
        ProgressBar pBar;
        unsigned int progInfoPosY, progInfoPosX;
        unsigned int pBarPosY, pBarPosX;

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
