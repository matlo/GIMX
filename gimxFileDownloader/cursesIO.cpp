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
    std::string fillString(int textLength, std::string filler)
    {
        std::string temp;
        for(int i = 0; i < textLength; ++i)
            temp += filler;
        return temp;
    }
    //-1 so 0 initialised
    int maxLines(int height, int padding) { return height - (padding *2); }
    int maxChars(int screenWidth, int padding) { return screenWidth - (padding *2); }


    WinData* newWinData(WINDOW* menu_win, std::string title, int height, int width, int startY, int startX, int padY, int padX, \
      bool drawBorder, int bordersWE, int bordersNS)
    {
        WinData* data = new WinData;

        data->title   = title;
        data->menuWin = menu_win;
        
        data->drawBorder = drawBorder;
        data->bordersWE  = bordersWE;
        data->bordersNS  = bordersNS;

        data->paddingY = padY;
        data->paddingX = padX;
        data->startY   = startY;
        data->startX   = startX;
        if(height == 0)
            data->height = getmaxy(data->menuWin);
        else
            data->height = height;
        if(data->width == 0)
            data->width = getmaxx(data->menuWin);
        else
            data->width  = width;
        
        return data;
    }
    

    SelectionMenu::SelectionMenu(WinData* windowsData, std::vector<std::string> choices) : Menus(windowsData)
    {
        this->choices = choices;
        numChoices    = choices.size();

        keypad(winData->menuWin, true);

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
        mvwprintw(winData->menuWin, y, winData->width -1, selected[index].c_str());
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
        werase(winData->menuWin);

        if(winData->drawBorder)
            box(winData->menuWin, winData->bordersWE, winData->bordersNS);
        reDrawAllCheckMarks();

        int titlePos = 1;
        if(winData->width > winData->title.length() )
            titlePos = centreText(winData->width, winData->title.length(), winData->paddingX);
        mvwprintw(winData->menuWin, 0, titlePos, winData->title.c_str());

        mvwprintw(winData->menuWin, winData->height -1, 0, "pg %i / %i", page, lastPage());
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
                    {
                        it = beginNEnd.second;
                    }
                    break;
                case seekOption::next:
                    if(it != beginNEnd.second)
                        ++it;
                    else
                        it = beginNEnd.first;
                    break;
            }

            wmove(winData->menuWin, currentLine(), winData->paddingX);
            wclrtoeol(winData->menuWin);
            wattron(winData->menuWin, A_REVERSE);
            mvwprintw(winData->menuWin, currentLine(), winData->paddingX, (*it).second.c_str());
            wattroff(winData->menuWin, A_REVERSE);

            reDrawCheckMark(highlight, currentLine());
            wrefresh(winData->menuWin);

            input = wgetch(winData->menuWin);
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
                input = wgetch(winData->menuWin);
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
            wrefresh(winData->menuWin);

            input = wgetch(winData->menuWin);
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
                wattron(winData->menuWin, A_REVERSE);
                mvwprintw(winData->menuWin, y, x, choices[i].c_str());
                wattroff(winData->menuWin, A_REVERSE);
            }
            else
                mvwprintw(winData->menuWin, y, x, "%s", choices[i].c_str());

            ++y;
        }
    }


    ttyProgressDialog::ttyProgressDialog(WinData* windowsData, std::string mssg) : Menus(windowsData)
    {
        this->message  = mssg;
    }

    void ttyProgressDialog::setDrawBorder(bool draw, int bordersWE, int bordersNS)
    {
        winData->drawBorder = draw;
        winData->bordersWE  = bordersWE;
        winData->bordersNS = bordersNS;
    }
    bool ttyProgressDialog::Update(double progress, std::string message)
    {
        /*Update ETA*/
        //TODO update ETA

        /*Update progress bar*/
        //TODO update progress bar
        //mvwprintw(winData->menuWin, 4, winData->paddingX +1, "%f", progress);
        return true;
    }
    void ttyProgressDialog::dialog()
    {
        /*Create title and message*/
        mvwprintw(winData->menuWin, 1, winData->paddingX, winData->title.c_str());
        mvwprintw(winData->menuWin, winData->paddingY, winData->paddingX, message.c_str());

        /*Create ETA*/
        //This will be on the 4th line => index 3
        //TODO add ETA

        /*Create progress bar*/
        //This will be on the 5th line => index 4
        //Generate bar text

        mvwprintw(winData->menuWin, 4, winData->paddingX, "[%s]", fillString( winData->width - (winData->paddingX *2) ).c_str());
    }

}