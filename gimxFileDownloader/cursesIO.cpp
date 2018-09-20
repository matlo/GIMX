/*
 * ncursesIO.cpp
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 10 Aug. 2018
 */

#include "include/cursesIO.h"

namespace EasyCurses
{
    //Miscellaneous
    int centreText(int screenWidth, int textLength, int padding) { return (screenWidth - (textLength + (padding *2) )) / 2; }
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
    }
    void eraseChunk(WINDOW* win, int startLine, int endLine, int startX, int endX, int amount)
    {
        if(startLine == endLine)
            eraseChunk(win, startLine, startX, endX - startX);
        else
            clrToEolFrom(win, startLine, startX); //first line

        for(int currentLine = startLine +1; currentLine < endLine; ++currentLine)
        {
            if(currentLine != endLine)
                clrToEolFrom(win, currentLine, 0); //Middle
        }
    }

    int maxLines(int height, int padding) { return height - (padding *2); }
    int maxChars(int screenWidth, int padding) { return screenWidth - (padding *2); }


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
        currentX   = this->startX;;
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

        /*mvwprintw(window, startY +1, startX, "width = %i barSize = %i currentX = %i diff = %i amount = %i prevAmount = %i\n" \
        "prefix = %s barChar = %s point = %s suffix = %s", COLS, barSize, currentX, diff, amount, prevAmount, prefix.c_str(), barChar, point, suffix.c_str());*/

        if(diff != 0 && amount < barSize)
            mvwprintw(window, startY, currentX, "%s", point);
    }


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
    

    SelectionMenu::SelectionMenu(WinData* windowsData, std::vector<std::string> choices, std::string title) : Menus(windowsData, title)
    {
        this->choices = choices;
        numChoices    = choices.size();

        keypad(winData->win, true);

        for(int i = 0; i < numChoices; ++i)
            selected[i +1] = blankMark;
    }
    void SelectionMenu::getResult(std::vector<int>& chosen)
    {
        for(auto option : selected)
        {
            if(option.second == checkMark)
            {
                //-1 so zero initialised
                chosen.push_back(option.first -1);
            }
        }
    }

    void SelectionMenu::reDrawCheckMark(int index, int y)
    {
        mvwprintw(winData->win, y, winData->width -1, selected[index].c_str());
    }
    void SelectionMenu::reDrawAllCheckMarks()
    {
        int y = winData->paddingY;
        for(int cIndex = pageBottom(); cIndex <= (page * _maxLines()) && cIndex <= numChoices; ++cIndex)
        {
            reDrawCheckMark(cIndex, y);
            ++y;
        }
    }
    void SelectionMenu::draw()
    {
        werase(winData->win);

        if(winData->drawBorder)
            box(winData->win, winData->bordersWE, winData->bordersNS);
        reDrawAllCheckMarks();

        int titlePos = 1;
        if(winData->width > title.length() )
            titlePos = centreText(winData->width, title.length(), winData->paddingX);
        mvwprintw(winData->win, 0, titlePos, title.c_str());

        mvwprintw(winData->win, winData->height -1, 0, "pg %i / %i", page, lastPage());
    }
    void SelectionMenu::calculatePage(seekOption seek)
    {
        //Check if we need to turn to next page
        //1st case: turn over last page to first, 2nd case: page up, 3rd case: turn over first page to last, 4th case: page down
        switch(seek)
        {
            case seekOption::next:
                if((highlight +1) > numChoices) //1st
                {
                    page = 1;
                    draw();
                    break;
                }
                else if(highlight == page * _maxLines()) //2nd
                {
                    ++page;
                    draw();
                    break;
                }

                break;

            case seekOption::back:
                if((highlight -1) == 0) //3rd
                {
                    page = lastPage();
                    draw();
                    break;
                }
                else if(highlight == pageBottom()) //4th
                {
                    --page;
                    draw();
                    break;
                }

                break;
        }
    }
    void SelectionMenu::prepareStr(std::string& text, int line)
    {
        if(text.length() > _maxChars())
        {
            int timesTrunc = 1;
            std::string firstSlice;
            std::string currentSlice;
            std::string nextSlice;

            do
            {
                currentSlice = nextSlice+ text.substr(_maxChars() *(timesTrunc -1), _maxChars() *timesTrunc);
                nextSlice = text.substr(currentSlice.size() -3, 3);
                currentSlice.replace(currentSlice.size() -3, 3, "...");

                truncated.insert(std::make_pair( line, currentSlice ));

                if(timesTrunc == 1)
                    firstSlice = currentSlice;
                ++timesTrunc;
            } while((_maxChars() *timesTrunc) < text.length());
            //Last truncation
            currentSlice  = nextSlice+ text.substr(_maxChars() *(timesTrunc -1), _maxChars() *timesTrunc);
            currentSlice += fillString(_maxChars() - currentSlice.length());
            truncated.insert(std::make_pair( line, currentSlice ));

            text = firstSlice;
        }
        else
            text += fillString(_maxChars() - text.length());
    }
    void SelectionMenu::truncNav(seekOption way, int& input)
    {
        while(true)
        {
            switch(way)
            {
                case seekOption::back:
                    if(it != beginNEnd.first)
                        --it;
                    else
                        it = beginNEnd.second;
                    break;

                case seekOption::next:
                    if(it != beginNEnd.second)
                        ++it;
                    else
                        it = beginNEnd.first;
                    break;
            }

            wmove(winData->win, currentLine(), winData->paddingX);
            wclrtoeol(winData->win);
            wattron(winData->win, A_REVERSE);
            mvwprintw(winData->win, currentLine(), winData->paddingX, (*it).second.c_str());
            wattroff(winData->win, A_REVERSE);

            reDrawCheckMark(highlight, currentLine());
            wrefresh(winData->win);

            input = wgetch(winData->win);
            switch(input)
            {
                case KEY_RIGHT:
                    way = seekOption::next;
                    break;

                case KEY_LEFT:
                    way = seekOption::back;
                    break;

                default:
                    return;
            }
        }
    }
    void SelectionMenu::menuLoop(int startChoice)
    {
        highlight = startChoice;
        page = 1;

        //If option name is too long, truncate it
        for(int i = 0; i < numChoices; ++i)
            prepareStr(choices[i], i);
        
        draw();

        int input;
        auto navigateTruncated = [&] (seekOption way) -> void {
            beginNEnd = truncated.equal_range(highlight -1);
            //Not a truncated string
            if(beginNEnd.first == beginNEnd.second)
            {
                //Due to feedback design of truncNav, need new input to prevent lock-up
                input = wgetch(winData->win);
                return;
            }

            //Upper-bound is one above last match to key
            --beginNEnd.second;
            it = beginNEnd.first;
            truncNav(way, input);
        };

        int choice = 0;
        while (true)
        {
            switch (input)
            {
                case KEY_UP:
                    calculatePage(seekOption::back);
                    //Gone past first page, head to last
                    if (highlight == 1)
                        highlight = numChoices;
                    else
                        --highlight;
                    break;
                    
                case KEY_DOWN:
                    calculatePage(seekOption::next);
                    //Gone past last page, head to first
                    if (highlight == numChoices)
                        highlight = 1;
                    else
                        ++highlight;
                    break;

                case KEY_RIGHT:
                    navigateTruncated(seekOption::next);
                    if(input == 10)
                        goto pick;
                    continue;

                case KEY_LEFT:
                    navigateTruncated(seekOption::back);
                    if(input == 10)
                        goto pick;
                    continue;

                case 10:
                    pick:
                    choice = highlight;
                    if(selected[highlight] == checkMark)
                        selected[highlight] = blankMark;
                    else
                        selected[highlight] = checkMark;
                    reDrawCheckMark(highlight, currentLine());
                    break;

                case 32:
                    //User done selecting
                    return;

                case 27:
                    //ESC hit
                    return;
            }

            menuHighlight();
            wrefresh(winData->win);

            input = wgetch(winData->win);
        }
    }

    void SelectionMenu::setDrawBorder(bool draw, int bordersWE, int bordersNS)
    {
        winData->drawBorder = draw;
        winData->bordersWE  = bordersWE;
        winData->bordersNS = bordersNS;
    }
    void SelectionMenu::menuHighlight()
    {
        int x, y;

        //Start postion of choice lables in window
        x = winData->paddingX;
        y = winData->paddingY;

        //Print options
        for (int i = pageBottom() -1; i < (page * _maxLines()) && i < numChoices; ++i)
        {
            //Highlight the present choice
            if (highlight == i + 1)
            {
                wattron(winData->win, A_REVERSE);
                mvwprintw(winData->win, y, x, choices[i].c_str());
                wattroff(winData->win, A_REVERSE);
            }
            else
                mvwprintw(winData->win, y, x, "%s", choices[i].c_str());

            ++y;
        }
    }


    ttyProgressDialog::ttyProgressDialog(WinData* windowsData, std::string title, std::string mssg) : Menus(windowsData, title),
      pBar(windowsData->win, 0, 0, 0), message(mssg), progInfo("")
    {
        //+2 to have space between eta data and message
        progInfoPosY = winData->paddingY +2;
        progInfoPosX = winData->paddingX;

        pBarPosY = progInfoPosY +1;
        pBarPosX = progInfoPosX;
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
        /*Create title and message*/
        {
            int x = centreText(winData->width, title.length(), winData->paddingX);
            mvwprintw(winData->win, 0, x, title.c_str());
        }
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