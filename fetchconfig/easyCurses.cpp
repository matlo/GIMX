/*
 * ncursesIO.cpp
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 10 Aug. 2018
 */

#include "include/easyCurses.h"

namespace EasyCurses
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Miscellaneous

    int roundUp(float n)
    {
        //First, check if float is integer/round number
            //Isolate decimal point
        int roundN = n;
                //Subtract round number from float => 2.6 - 2 OR 3.0 - 3
        float decimal = n - roundN;

            //Check if decimal is .0 to see if n is an interger
        if(decimal == .0f)
            return roundN;
        else
            return roundN +1;
    }

    int centreText(int windowWidth, int textLength)
    {
        //Math works best with even numbers
        if((windowWidth % 2) != 0)
            windowWidth -= 1;
        return (windowWidth - textLength) / 2;
    }

    std::string fillString(int textLength, char filler)
    {
        std::string temp;
        for(int i = 0; i < textLength; ++i)
            temp += filler;
        return temp;
    }

    void clrToEolFrom(WINDOW* win, int y, int x)
    {
        wmove(win, y, x);
        wclrtoeol(win);
    }
    void eraseChunk(WINDOW* win, int y, int x, int amount)
    {
        mvwprintw(win, y, x, fillString(amount).c_str());
        wmove(win, y, x);
    }
    void eraseChunk(WINDOW* win, int startY, int endY, int startX, int endX)
    {
        for(int currentLine = startY; currentLine <= endY; ++currentLine)
        {
            eraseChunk(win, currentLine, startX, endX - startX);
        }
    }

    int maxLines(int height, int padding) { return height - (padding *2); }
    int maxChars(int screenWidth, int padding) { return screenWidth - (padding *2); }

    namespace TextFormat
    {
        void overflow(std::string text, int maxLength, LineEnds& markers, bool markVirtEnd)
        {
            //Scan to newline, then check if text is too long. If it is, mark a new line
            std::array<int, 2> currentRange = { 1, 0 };
            size_t lineStart = currentRange.front();
            size_t lineEnd   = currentRange.back();

            auto lineLength = [&] () -> int {
                return lineEnd - lineStart;
            };

            size_t virtEnd;
            auto setVirtEnd = [&] () -> void {
                virtEnd = (lineStart + maxLength) -1; //-1 so zero-initialised
            };

            markers.clear();
            while(true)
            {
                lineEnd = text.find("\n", lineStart -1); //-1 to avoid skipping first char

                if(lineEnd == std::string::npos)
                    lineEnd = text.length();

                if(lineLength() > maxLength)
                {
                    setVirtEnd();
                    while(virtEnd < lineEnd)
                    {
                        int marked = virtEnd;
                        if(markVirtEnd)
                            marked = -virtEnd; //Mark softwrapping with negative numbers
                        markers.push_back(marked);
                        lineStart = (virtEnd) +1;
                        setVirtEnd();
                    }
                }
                else
                {
                    markers.push_back(lineEnd);
                    lineStart = lineEnd +2; //+2 to skip newline and start new
                    if(lineStart >= text.length())
                        break;
                }
            }
        }

        unsigned int isOverflow(LineEnds markers, int index)
        {
            int endPoint = markers[index];
            if(index == 0 && endPoint < 0)
                return 1;
            else if(index == 0 && endPoint > 0)
                return 0;

            int prevEndPoint = markers[index -1];

            if(endPoint < 0)
            {
                if(prevEndPoint < 0)
                    return 2;
                return 3;
            }
            else if(prevEndPoint < 0)
                return 4;

            return 0;
        }

        void pageFormat(int maxLines, LineEnds markers, TextLayout& pageLayout, TextLayout& overflowLayout)
        {
            int numLines = markers.size();
            int numLinesSkipped = 0;
            int page = 0;
            int endPoint;
            int startOfOverflow;
            bool lastLineOverflowed = false;

            auto setEndPoint = [&] (int index) -> void {
                endPoint = markers[index];
                if(lastLineOverflowed)
                {
                    endPoint = -endPoint;
                    lastLineOverflowed = false;
                }
                else if(endPoint < 0)
                    endPoint *= -1;
            };

            for(int l = 0; l < numLines; ++l)
            {
                int res = TF::isOverflow(markers, l);
                switch(res)
                {
                    case 0:
                    case 1:
                    case 3:
                        setEndPoint(l);
                        pageLayout.insert(std::pair<int, int>(page, endPoint));
                        if(res == 1 || res == 3)
                        {
                            //Start filling out new overflowLayout
                            startOfOverflow = l - numLinesSkipped;
                            overflowLayout.insert(std::pair<int, int>(startOfOverflow, endPoint));
                        }
                        break;

                    case 2:
                    case 4:
                        ++numLinesSkipped;
                        setEndPoint(l);

                        overflowLayout.insert(std::pair<int, int>(startOfOverflow, endPoint));
                        if(res == 4)
                            lastLineOverflowed = true;
                        break;
                }

                if(l == ((maxLines * (page +1)) -1) + numLinesSkipped)
                    ++page;
            }
        }

    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Widgets

    ProgressBar::ProgressBar(WINDOW* win, int size, int startY, int startX, std::string prefix, char barChar, char point, std::string suffix)
    {
        init(win, size, startY, startX, prefix, barChar, point, suffix);
    }
    void ProgressBar::init(WINDOW* win, int size, int startY, int startX, std::string prefix, char barChar, char point, std::string suffix)
    {
        /*Setup values and generate progress bar*/
        this->prefix     = prefix;
        this->suffix     = suffix;
        this->barChar[0] = barChar;
        this->point[0]   = point;
        this->barChar[1] = 0;
        this->point[1]   = 0;
        setSize(size);
        prevAmount = 0;

        window = win;
        this->startY = startY;
        this->startX = startX;
        currentX = this->startX;
    }
    void ProgressBar::reset()
    {
        prevAmount = 0;
        currentX   = this->startX;
    }
    void ProgressBar::reset(WINDOW* win, int size, int startY, int startX, std::string prefix, char barChar, char point, std::string suffix)
    {
        init(win, size, startY, startX, prefix, barChar, point, suffix);
    }

    void ProgressBar::first()
    {
        //-1 so zero initialised like progress
        mvwprintw(window, startY, startX, "%s%s%s%s", prefix.c_str(), point, fillString(barSize -1).c_str(), suffix.c_str());
        currentX += prefix.length();
    }
    void ProgressBar::update(double progress)
    {
        /*Only print difference between the last and future render*/
        //Progress needs to be a fraction to convert progress % into % of progress bar
        int amount = (progress /100) * barSize;
        int diff   = amount - prevAmount;

        mvwprintw(window, startY, currentX, fillString(diff, barChar[0]).c_str());

        currentX += diff;
        prevAmount = amount;

        if(diff != 0 && amount < barSize)
            mvwprintw(window, startY, currentX, "%s", point);
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Menus

    WinData* newWinData(WINDOW* window, int height, int width, int startY, int startX, int padY, int padX, \
      bool drawBorder, int bordersWE, int bordersNS)
    {
        WinData* data = new WinData;

        data->win = window;

        data->drawBorder = drawBorder;
        data->bordersWE  = bordersWE;
        data->bordersNS  = bordersNS;

        data->paddingY = padY;
        data->paddingX = padX;
        data->startY   = startY;
        data->startX   = startX;
        if(height == 0)
            data->height = getmaxy(data->win);
        else
            data->height = height;
        if(width == 0)
            data->width  = getmaxx(data->win);
        else
            data->width  = width;

        return data;
    }


    void Menus::drawTitle()
    {
        /*Create title and message*/
        int padding = centreText(winData->width, title.length());
        mvwprintw(winData->win, 0, padding, title.c_str());
        wrefresh(winData->win);
    }

    void Menus::drawFrame()
    {
        if(winData->drawBorder)
            box(winData->win, winData->bordersWE, winData->bordersNS);
        wrefresh(winData->win);
    }


    BasicMenu::BasicMenu(std::string lines, WinData* windowsData, std::string title) : Menus(windowsData, title)
    {
        text = lines;
        TF::overflow(lines, _maxChars(), lineFormat);
        numLines    = lineFormat.size();

        keypad(winData->win, true);
    }

    std::map<int, int> BasicMenu::keyBindings = {
        { KEY_NPAGE, NavContent::pageUp }, { KEY_PPAGE, NavContent::pageDown },
        { 27, NavContent::finish }
    }; //27 => ESC

    int BasicMenu::getInput()
    {
        return wgetch(winData->win);
    }

    int BasicMenu::mapInput(int input)
    {
        auto res = keyBindings.find(input);
        if(res != keyBindings.end())
            return res->second;
        return NavContent::null;
    }

    void BasicMenu::setCustomAction(F cusAct)
    {
        this->cusAct = cusAct;
    }

    BasicMenu::F& BasicMenu::getCustomAction()
    {
        return cusAct;
    }

    void BasicMenu::calculatePage(int seek)
    {
        int startPage = page;

        switch(seek)
        {
            /*
             * [pageUp]
             *     1st case: turn over first page to last, 2nd case: page up,
             * [pageDown]
             *     3rd case: turn over last page to first, 4th case: page down
             */
            case NavContent::pageUp:
                if(page == 0) //1st
                    page = lastPage() -1;
                else //2nd
                    --page;
                break;

            case NavContent::pageDown:
                if(page == lastPage() -1) //3rd
                    page = 0;
                else //4th
                    ++page;
                break;

            break;
        }

        if(startPage != page)
            setUpdate();
    }

    void BasicMenu::inputHandling(int& input)
    {
        input = mapInput(getInput());

        if(input == NavContent::custom)
        {
            if(cusAct())
                update();
        }
    }

    void BasicMenu::printStyle(int& x, int& y)
    {
        for(int l = pageTop(); l <= pageBottom() && l < numLines; ++l)
        {
            int endPoint  = lineFormat[l]; //-1 so zero-initialised
            int lineStart = (l == 0 ? 0 : lineFormat[l - 1] );

            wmove(winData->win, y, x);
            for(int c = lineStart; c < endPoint; ++c)
            {
                if(text.compare(c, 1, "\n") == 0)
                    continue;
                waddch(winData->win, text[c]);
            }
            ++y;
        }
    }

    void BasicMenu::drawContent()
    {
        int x, y;

        //Start postion of content in window
        x = winData->paddingX;
        y = winData->paddingY;

        eraseChunk(winData->win, y, _maxLines() +y, x, _maxChars() +x);

        printStyle(x, y);
        wrefresh(winData->win);
    }

    void BasicMenu::drawPageNumber()
    {
        mvwprintw(winData->win, winData->height -1, 1, "pg %i / %i", page +1, lastPage());
        wrefresh(winData->win);
    }

    bool BasicMenu::doUpdate()
    {
        bool old = changed;
        changed = false;
        return old;
    }

    void BasicMenu::update()
    {
        drawContent();
        drawFrame();
        drawPageNumber();
        drawTitle();
        wrefresh(winData->win);
    }

    void BasicMenu::menuLoop()
    {
        page = 0;
        update();
        wrefresh(winData->win);

        int input = NavContent::null;
        do
        {
            inputHandling(input);
            calculatePage(input);
            if(doUpdate())
                update();
        } while(input != NavContent::finish);
    }

    void BasicMenu::setDrawBorder(bool draw, int bordersWE, int bordersNS)
    {
        winData->drawBorder = draw;
        winData->bordersWE  = bordersWE;
        winData->bordersNS = bordersNS;
    }


    SelectionMenu::SelectionMenu(std::string text, WinData* windowsData, std::string title) : BasicMenu(text, windowsData, title)
    {
        this->text = text;

        {
            TF::LineEnds lineFormat;
            TF::overflow(text, _maxChars(), lineFormat, true);
            TF::pageFormat(_maxLines(), lineFormat, pageLayout, overflowLayout);
        }
        numLines = pageLayout.size();

        for(int i = 0; i < numLines; ++i)
            selected[i] = false;

        keyBindings[KEY_RIGHT] = NavContent::right;
        keyBindings[KEY_LEFT]  = NavContent::left;
        keyBindings[KEY_UP]    = NavContent::lineUp;
        keyBindings[KEY_DOWN]  = NavContent::lineDown;
        keyBindings[10]        = 10;

        keypad(winData->win, true);
    }

    void SelectionMenu::getResult(std::vector<int>& chosen)
    {
        for(auto option : selected)
        {
            if(option.second == true)
                chosen.push_back(option.first);
        }
    }

    void SelectionMenu::drawCheckMark(int index, int y)
    {
        mvwprintw(winData->win, y, winData->width -1, (selected[index] ? checkMark : blankMark ).c_str());
        wrefresh(winData->win);
    }

    void SelectionMenu::drawAllCheckMarks()
    {
        int y = winData->paddingY;
        for(int cIndex = pageTop(); cIndex <= pageBottom() && cIndex < numLines; ++cIndex)
        {
            drawCheckMark(cIndex, y);
            ++y;
        }
    }

    void SelectionMenu::calculatePage(int seek)
    {
        //Deals with page up and down keys
        BasicMenu::calculatePage(seek);
        if(doUpdate())
        {
            updateLineTrackers(pageTop());
            setUpdate();
            return;
        }


        int startPage = page;

        //Check if we need to turn to next page
        //1st case: turn over last page to first, 2nd case: page up/next, 3rd case: turn over first page to last, 4th case: page down/back
        switch(seek)
        {
            case NavContent::lineDown:
                setUpdate();
                updateLineTrackers(highlight +1);

                if(highlight == numLines) //1st
                {
                    page = 0;
                    updateLineTrackers(pageTop());
                    break;
                }
                else if(highlight > pageBottom()) //2nd
                {
                    ++page;
                    updateLineTrackers(pageTop());
                    break;
                }

                break;

            case NavContent::lineUp:
                setUpdate();
                updateLineTrackers(highlight -1);

                if(highlight == -1) //3rd
                {
                    page = lastPage() -1;
                    updateLineTrackers(numLines -1);
                    break;
                }
                else if(highlight < pageTop()) //4th
                {
                    --page;
                    updateLineTrackers(pageBottom());
                    break;
                }

                break;
        }

        if(startPage != page)
            setUpdate();
    }

    void SelectionMenu::navTrunc(int input)
    {
        if(overflowLine == overflowLayout.end())
            return;


        bool changed = false;

        TF::TextLayout::iterator first, last;
        {
            auto range  = overflowLayout.equal_range(highlight);
            first  = range.first;
            last   = range.second;
            --last;
        }

        switch(input)
        {
        case NavContent::left:
            if(overflowLine != first)
            {
                changed = true;
                --overflowLine;
            }
            break;

        case NavContent::right:
            if(overflowLine != last)
            {
                changed = true;
                ++overflowLine;
            }
            break;
        }

        if(changed)
        {
            eraseChunk(winData->win, currentLine(), winData->paddingX, _maxChars());
            wattron(winData->win, A_REVERSE);

            int lineStart;
            if(overflowLine == first)
            {
                int increment = ( highlight - (page * _maxLines()) ) -1;
                auto lastLine = pageLayout.lower_bound(page);
                if(increment == -1)
                    --lastLine;
                else
                {
                    for(int i = 0; i < increment; ++i)
                    {
                        ++lastLine;
                    }
                }
                lineStart = lastLine->second;
            }
            else
            {
                auto temp = overflowLine;
                --temp;
                lineStart = temp->second;
            }
            for(int c = lineStart; c < overflowLine->second; ++c)
            {
                if(text.compare(c, 1, "\n") == 0)
                    continue;
                waddch(winData->win, text[c]);
            }

            wattroff(winData->win, A_REVERSE);
        }

        wrefresh(winData->win);
    }

    void SelectionMenu::inputHandling(int& input)
    {
        BasicMenu::inputHandling(input);


        switch (input)
        {
            case NavContent::right:
                navTrunc(input);
                break;

            case NavContent::left:
                navTrunc(input);
                break;

            case 10:
                if(selected[highlight] == true)
                    selected[highlight] = false;
                else
                    selected[highlight] = true;
                drawCheckMark(highlight, currentLine());
                break;

            case NavContent::finish:
                //User done selecting
                return;
        }
    }

    void SelectionMenu::drawFrame()
    {
        BasicMenu::drawFrame();

        drawAllCheckMarks();
        wrefresh(winData->win);
    }

    void SelectionMenu::update()
    {
        BasicMenu::update();

        drawAllCheckMarks();
    }

    void SelectionMenu::menuLoop(int startChoice)
    {
        updateLineTrackers(startChoice);

        BasicMenu::menuLoop();
    }

    void SelectionMenu::printStyle(int& x, int& y)
    {
        auto range = pageLayout.equal_range(page);

        {
            int l = 0;
            int endPoint;
            int lineStart;
            if(page == 0)
                lineStart = 0;
            else
            {
                auto temp = pageLayout.equal_range(page -1).second;
                --temp;
                lineStart = temp->second;
            }
            for(auto it = range.first; it != range.second; ++it)
            {
                endPoint = it->second;
                if(endPoint < 0)
                {
                    endPoint *= -1;
                    int thisLine = (l -1)  + ( page * _maxLines() );
                    auto temp = overflowLayout.upper_bound(thisLine);
                    --temp;
                    lineStart = temp->second;
                }

                wmove(winData->win, y, x);
                for(int c = lineStart; c < endPoint; ++c)
                {
                    if(text.compare(c, 1, "\n") == 0)
                        continue;

                    //Highlight the present choice
                    if(highlight == l + (_maxLines() * page))
                    {
                        wattron(winData->win, A_REVERSE);
                        waddch(winData->win, text[c]);
                        wattroff(winData->win, A_REVERSE);
                    }
                    else
                    {
                        waddch(winData->win, text[c]);
                    }
                }

                ++y;
                ++l;
                lineStart = endPoint;
            }
        }
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Dialogs

    ttyProgressDialog::ttyProgressDialog(WinData* windowsData, std::string title, std::string mssg)\
      : Menus(windowsData, title), pBar(windowsData->win, 0, 0, 0)
    {
        //+2 to have space between eta data and message
        progInfoPosY = winData->paddingY +2;
        progInfoPosX = winData->paddingX;

        pBarPosY = progInfoPosY +1;
        pBarPosX = progInfoPosX;

        message = mssg;

        progInfo = "";
    }

    void ttyProgressDialog::setDrawBorder(bool draw, int bordersWE, int bordersNS)
    {
        winData->drawBorder = draw;
        winData->bordersWE  = bordersWE;
        winData->bordersNS  = bordersNS;
    }

    bool ttyProgressDialog::update(double progress, std::string mssg)
    {
        if(!mssg.empty())
        {
            int diff = message.length() - mssg.length();
            if(diff > 0)
                eraseChunk(winData->win, winData->paddingY, winData->paddingX +mssg.length(), diff);

            message = mssg;
            mvwprintw(winData->win, winData->paddingY, winData->paddingX, message.c_str());
        }

        /*Update ETA*/
        //TODO add ETA
        mvwprintw(winData->win, progInfoPosY, progInfoPosX, "%*.2f%%", 6, progress);

        /*Update progress bar*/
        pBar.update(progress);

        wrefresh(winData->win);
        return true;
    }
    void ttyProgressDialog::dialog()
    {
        drawFrame();
        drawTitle();
        mvwprintw(winData->win, winData->paddingY, winData->paddingX, message.c_str());

        /*Create ETA*/
        //This will be on the 4th line => index 3
        //TODO add ETA
        mvwprintw(winData->win, progInfoPosY, progInfoPosX, progInfo.c_str());

        /*Create space for progress bar*/
        //This will be on the 5th line => index 4
        pBar.setStartCoordinates(pBarPosY, pBarPosX);
        pBar.setSize(winData->width - (winData->paddingX *2));
        pBar.first();

        wrefresh(winData->win);
    }

}
