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

    unsigned roundUp(float n)
    {
        //First, check if float is integer/round number
            //Isolate decimal point
        unsigned roundN = n;
                //Subtract round number from float => 2.6 - 2 OR 3.0 - 3
        float decimal = n - roundN;

            //Check if decimal is .0 to see if n is an interger
        if(decimal == .0f)
            return roundN;
        else
            return roundN +1;
    }

    unsigned centreText(unsigned windowWidth, unsigned textLength)
    {
        //Math works best with even numbers
        if((windowWidth % 2) != 0)
            windowWidth -= 1;
        return (windowWidth - textLength) / 2;
    }

    std::string fillString(unsigned textLength, char filler)
    {
        std::string temp;
        for(unsigned i = 0; i < textLength; ++i)
            temp += filler;
        return temp;
    }

    void clrToEolFrom(WINDOW* win, unsigned y, unsigned x)
    {
        wmove(win, y, x);
        wclrtoeol(win);
    }
    void eraseChunk(WINDOW* win, unsigned y, unsigned x, unsigned amount)
    {
        mvwprintw(win, y, x, fillString(amount).c_str());
        wmove(win, y, x);
    }
    void eraseChunk(WINDOW* win, unsigned startY, unsigned endY, unsigned startX, unsigned endX)
    {
        for(unsigned currentLine = startY; currentLine <= endY; ++currentLine)
        {
            eraseChunk(win, currentLine, startX, endX - startX);
        }
    }

    unsigned maxLines(unsigned height, unsigned padding) { return height - (padding *2); }
    unsigned maxChars(unsigned screenWidth, unsigned padding) { return screenWidth - (padding *2); }

    namespace TextFormat
    {
        void overflow(std::string text, unsigned maxLength, LineEnds& lineFormat, OverFlow& oFLayout, bool wrap)
        {
            //Each time this is run, information is recalculated.
            lineFormat.clear();
            oFLayout.clear();

            size_t numChars;
            size_t   start     = 0;
            size_t   linePos   = 0;
            size_t currentLine = 0;

            LineEnds oFlow;

            while(linePos != text.length() && (linePos +1) != text.length())
            { //2nd case is when last char is new line
                {
                    size_t endPoint;

                    //Set end points
                    linePos = text.find("\n", linePos + (linePos == 0 ? 0 : 1));

                    if(linePos == std::string::npos)
                        linePos = text.length();

                    if(linePos > (maxLength + start))
                    {
                        //Fill out overflow information
                        size_t virtStart = start + maxLength;
                        size_t virtEnd;

                        auto commit = [&]() -> void {
                            if(wrap)
                                oFlow.push_back(std::make_pair(virtEnd, numChars));
                            else
                                oFLayout.insert(std::make_pair( currentLine,
                                  std::make_pair(virtEnd, numChars) ));

                        };


                        while(true)
                        {
                            virtEnd = virtStart + maxLength;

                            if(virtEnd > linePos)
                            {
                                virtEnd  = linePos;
                                numChars = linePos - virtStart;

                                commit();
                                break;
                            }
                            else
                            {
                                numChars = maxLength;

                                commit();

                                if(virtEnd == linePos)
                                    break;
                            }

                            virtStart = virtEnd;
                        }

                        numChars = maxLength +1;
                        endPoint = start + maxLength;
                        start    = linePos +1;
                    }
                    else
                    {
                        numChars = linePos - start;

                        start    = linePos +1;
                        endPoint = linePos;
                    }

                    lineFormat.push_back(std::make_pair(endPoint, numChars));
                    if(wrap)
                    {
                        for(auto line : oFlow)
                        {
                            size_t ep = line.first;
                            unsigned chars = line.second;
                            lineFormat.push_back(std::make_pair(ep, chars));
                        }
                        oFlow.clear();
                    }
                }

                ++currentLine;
            }
        }

        void overflow(std::string text, unsigned maxLength, LineEnds& lineFormat)
        {
            OverFlow placeHolder;

            overflow(text, maxLength, lineFormat, placeHolder, true);
        }
    }



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Widgets

    ProgressBar::ProgressBar(WINDOW* win, unsigned size, unsigned startY, unsigned startX, std::string prefix, std::string suffix, char barChar, char point)
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
        unsigned amount = (progress /100) * barSize;
        unsigned diff   = amount - prevAmount;

        mvwprintw(window, startY, currentX, fillString(diff, barChar[0]).c_str());

        currentX += diff;
        prevAmount = amount;

        if(diff != 0 && amount < barSize)
            mvwprintw(window, startY, currentX, "%s", point);
    }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Menus

    WinData* newWinData(WINDOW* window, unsigned height, unsigned width, unsigned startY, unsigned startX, unsigned padY, unsigned padX, \
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
        unsigned padding = centreText(winData->width, title.length());
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
        TF::overflow(lines, _maxChars(), pageLayout);
        numLines = pageLayout.size();

        keypad(winData->win, true);
        keyBindings = {
            { KEY_NPAGE, NavContent::pageUp }, { KEY_PPAGE, NavContent::pageDown },
            { 27, NavContent::finish }
        }; //27 => ESC

        this->cusAct = [](void) -> bool {
            return false;
        };
    }

    unsigned BasicMenu::getInput()
    {
        return wgetch(winData->win);
    }

    NavContent BasicMenu::mapInput(unsigned rawInput)
    {
        auto res = keyBindings.find(rawInput);
        if(res != keyBindings.end())
            return res->second;
        return NavContent::null;
    }

    void BasicMenu::calculatePage(NavContent seek)
    {
        unsigned startPage = page;

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

            default:
                break;
        }

        if(startPage != page)
            setUpdate();
    }

    void BasicMenu::inputHandling(NavContent& input)
    {
        input = mapInput(getInput());

        if(input == NavContent::custom)
        {
            if(cusAct())
                update();
        }
    }

    void BasicMenu::printStyle()
    {
        unsigned x, y;
        x = winData->paddingX;
        y = winData->paddingY;

        for(unsigned l = pageTop(); l <= pageBottom() && l < numLines; ++l)
        {
            auto info        = pageLayout[l];
            unsigned numChars     = (info.second == _maxChars() +1? _maxChars() : info.second); //-1 so zero-initialised
            size_t lineStart = info.first - numChars;

            wmove(winData->win, y, x);
            waddstr(winData->win, text.substr(lineStart, numChars).c_str());
            ++y;
        }
    }

    void BasicMenu::drawContent()
    {
        unsigned x, y;

        //Start postion of content in window
        x = winData->paddingX;
        y = winData->paddingY;

        eraseChunk(winData->win, y, _maxLines() +y, x, _maxChars() +x);

        printStyle();
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

        NavContent input = NavContent::null;
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

        TF::overflow(text, _maxChars(), pageLayout, oFLayout);
        oFLine  = 0;
        numLines = pageLayout.size();

        for(unsigned i = 0; i < numLines; ++i)
            selected[i] = false;

        keyBindings[KEY_RIGHT] = NavContent::right;
        keyBindings[KEY_LEFT]  = NavContent::left;
        keyBindings[KEY_UP]    = NavContent::lineUp;
        keyBindings[KEY_DOWN]  = NavContent::lineDown;
        keyBindings[10]        = NavContent::select; //Enter key

        keypad(winData->win, true);
    }

    void SelectionMenu::getResult(std::vector<unsigned>& chosen)
    {
        for(auto option : selected)
        {
            if(option.second == true)
                chosen.push_back(option.first);
        }
    }

    void SelectionMenu::updateLineTrackers(unsigned n)
    {
        highlight = n;
        oFLine   = 0;
    }

    void SelectionMenu::drawCheckMark(unsigned index, unsigned y)
    {
        mvwprintw(winData->win, y, winData->width -1, (selected[index] ? checkMark : blankMark ).c_str());
        wrefresh(winData->win);
    }

    void SelectionMenu::drawAllCheckMarks()
    {
        unsigned y = winData->paddingY;
        for(unsigned cIndex = pageTop(); cIndex <= pageBottom() && cIndex < numLines; ++cIndex)
        {
            drawCheckMark(cIndex, y);
            ++y;
        }
    }

    void SelectionMenu::calculatePage(NavContent seek)
    {
        //Deals with page up and down keys
        BasicMenu::calculatePage(seek);
        if(doUpdate())
        {
            updateLineTrackers(pageTop());
            setUpdate();
            return;
        }

        unsigned startPage = page;

        /*
         * Check if we need to turn to next page:
         * 1st case: turn over last page to first, 2nd case: page up/next,
         * 3rd case: lineDown,
         * 4th case: turn over first page to last, 5th case: page down/back,
         * 6th case: lineUp
         */
        switch(seek)
        {
            case NavContent::lineDown:
                setUpdate();

                if((highlight +1) == numLines) //1st
                {
                    page = 0;
                    updateLineTrackers(0);
                    break;
                }
                else if((highlight +1) > pageBottom()) //2nd
                {
                    ++page;
                    updateLineTrackers(pageTop());
                    break;
                }
                else //3rd
                    updateLineTrackers(highlight +1);
                break;

            case NavContent::lineUp:
                setUpdate();

                if(highlight == 0) //4th
                {
                    page = lastPage() -1;
                    updateLineTrackers(pageBottom());
                }
                else if((highlight -1) < pageTop()) //5th
                {
                    --page;
                    updateLineTrackers(pageBottom());
                }
                else //6th
                    updateLineTrackers(highlight -1);
                break;

            default:
                break;
        }

        if(startPage != page)
            setUpdate();
    }

    void SelectionMenu::navTrunc(NavContent input)
    {
        if(pageLayout[highlight].second != _maxChars() +1)
            return;

        bool changed      = false;
        unsigned numChars = 0;
        size_t lineStart  = 0;

        unsigned first = 0;
        unsigned last  = oFLayout.count(highlight);

        auto set = [&]() -> void {
            auto temp = oFLayout.lower_bound(highlight);
            for(unsigned count = 1; count < oFLine; ++count)
            {
                ++temp;
            }
            numChars = (temp->second).second;
            lineStart = ((temp->second).first) - numChars;
        };

        switch(input)
        {
        case NavContent::left:
            if(oFLine != 0 && (oFLine -1) > first)
            {
                changed = true;
                --oFLine;
                set();
            }
            else if((oFLine -1) < first)
                break;
            else // oFLine == 0
            {
                changed = true;
                oFLine  = 0;

                auto info = pageLayout[highlight];
                numChars  = (info.second == _maxChars() +1? _maxChars() : info.second);
                lineStart = info.first - numChars;
            }
            break;

        case NavContent::right:
            if(oFLine == first)
            {
                changed = true;
                set();
                ++oFLine;
            }
            else if((oFLine +1) <= last)
            {
                changed = true;
                ++oFLine;
                set();
            }
            break;

        default:
            break;
        }

        if(changed)
        {
            eraseChunk(winData->win, yCoord(), winData->paddingX, _maxChars());
            wattron(winData->win, A_REVERSE);
            waddstr(winData->win, text.substr(lineStart, numChars).c_str());
            wattroff(winData->win, A_REVERSE);
        }

        wrefresh(winData->win);
    }

    void SelectionMenu::inputHandling(NavContent& input)
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

            case NavContent::select:
                if(selected[highlight] == true)
                    selected[highlight] = false;
                else
                    selected[highlight] = true;
                drawCheckMark(highlight, yCoord());
                break;

            case NavContent::finish:
                //User done selecting
                return;

            default:
                break;
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

    void SelectionMenu::menuLoop(unsigned startChoice)
    {
        updateLineTrackers(startChoice);

        BasicMenu::menuLoop();
    }

    void SelectionMenu::printStyle()
    {
        unsigned x, y;
        //Start postion of content in window
        x = winData->paddingX;
        y = winData->paddingY;

        for(unsigned l = pageTop(); l <= pageBottom() && l < numLines; ++l)
        {
            auto info        = pageLayout[l];
            unsigned numChars     = (info.second == _maxChars() +1? _maxChars() : info.second);
            size_t lineStart = info.first - numChars;

            wmove(winData->win, y, x);
            //Highlight the present choice
            if(highlight == l)
            {
                wattron(winData->win, A_REVERSE);
                waddstr(winData->win, text.substr(lineStart, numChars).c_str());
                wattroff(winData->win, A_REVERSE);
            }
            else
            {
                waddstr(winData->win, text.substr(lineStart, numChars).c_str());
            }
            ++y;
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
            unsigned diff = message.length() - mssg.length();
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
        pBar.setStartCoords(pBarPosY, pBarPosX);
        pBar.setSize(winData->width - (winData->paddingX *2));
        pBar.first();

        wrefresh(winData->win);
    }

}
