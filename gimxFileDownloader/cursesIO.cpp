/*
 * ncursesIO.cpp
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 10 Aug. 2018
 */

#include "include/cursesIO.h"

Menu::Menu(WINDOW* menu_win, std::vector<std::string> choices, std::string title, int pady, int padx) \
    : Menu(menu_win, LINES, COLS, 0, 0, choices, title, pady, padx)
{
    getmaxyx(menu_win, height, width);
    starty = 0;
    startx = 0;
}
Menu::Menu(WINDOW* menu_win, int height, int width, int starty, int startx, std::vector<std::string> choices, std::string title, int pady=2, int padx=2)
{
    drawBorder = true;
    bordersWE  = 0;
    bordersNS  = 0;

    paddingy = pady;
    paddingx = padx;
    this->starty = starty;
    this->startx = startx;
    this->height = height;
    this->width  = width;

    this->title   = title;
    this->menuWin = menu_win;
    page = 1;

    this->choices = choices;
    numChoices    = choices.size();

    keypad(this->menuWin, true);
}

void Menu::draw()
{
    werase(menuWin);

    if (drawBorder)
        box(menuWin, bordersWE, bordersNS);

    mvwprintw(menuWin, 0, 1, title.c_str());
    std::string pageNum = "pg " + std::to_string(page);
    mvwprintw(menuWin, 0, width - pageNum.length(), pageNum.c_str());
}
void Menu::calculatePage(seekOption seek)
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
            else if(highlight == page * maxLines) //2nd
            {
                ++page;
                draw();
                break;
            }

            break;

        case seekOption::back:
            if((highlight -1) == 0) //3rd
            {
                page = ceil( float(numChoices) / float(maxLines) );
                draw();
                break;
            }
            else if(highlight -1 == (page -1) * maxLines) //4th
            {
                --page;
                draw();
                break;
            }

            break;
    }
}
void Menu::truncStr(std::string& text, int line)
{
    if(text.length() > maxChars)
    {
        int timesTrunc = 1;
        std::string firstSlice;
        std::string currentSlice;
        std::string nextSlice;

        do
        {
            currentSlice = nextSlice+ text.substr(maxChars *(timesTrunc -1), maxChars *timesTrunc);
            nextSlice = text.substr(currentSlice.size() -3, 3);
            currentSlice.replace(currentSlice.size() -3, 3, "...");

            truncated.insert(std::make_pair( line, currentSlice ));

            if(timesTrunc == 1)
                firstSlice = currentSlice;
            ++timesTrunc;
        } while((maxChars *timesTrunc) < text.length());
        //Last truncation
        currentSlice = nextSlice+ text.substr(maxChars *(timesTrunc -1), maxChars *timesTrunc);
        truncated.insert(std::make_pair( line, currentSlice ));

        text = firstSlice;
    }
}
//For naviagting truncated lines
void Menu::truncNav(seekOption way, int& input)
{
    wattron(menuWin, A_REVERSE);

    switch(way)
    {
        case seekOption::back:
            if(it != beginNend.first)
                --it;
            break;
        case seekOption::next:
            if(it != beginNend.second)
                ++it;
            break;
    }

    wmove(menuWin, highlight -1 +paddingy - (maxLines * (page -1)), paddingx);
    wclrtoeol(menuWin);
    mvwprintw(menuWin, highlight -1 +paddingy - (maxLines * (page -1)), paddingx, (*it).second.c_str());
    wrefresh(menuWin);

    wattroff(menuWin, A_REVERSE);
}
int Menu::menuLoop(int startChoice)
{
    highlight = startChoice;

    //-1 so 0 initialised
    maxLines = (height - (paddingy)) -1;
    maxChars = width - (paddingx *2);
    page = 1;

    //If option name is too long, truncate it
    for(int i = 0; i < choices.size(); ++i)
        truncStr(choices[i], i);

    draw();
    menuHighlight();

    int input;
    auto navigateTruncated = [&] (seekOption way) -> void {
        beginNend = truncated.equal_range(highlight -1);
        //Not a truncated string
        if(beginNend.first == beginNend.second)
            return;

        it = beginNend.first;
        truncNav(way, input);
    };

    int choice = 0;
    while (true)
    {
        input = wgetch(menuWin);
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
            {
                navigateTruncated(seekOption::next);
                if(input == 10)
                    goto chose;
                continue;
            }
            case KEY_LEFT:
            {
                navigateTruncated(seekOption::back);
                if(input == 10)
                    goto chose;
                continue;
            }
            case 10:
                chose:
                choice = highlight;
                break;
            case 27:
                //ESC hit
                return 0;
        }

        /*Check if already chosen and render check marks*/
        /*for(int choice : chosen)
        {
            //Already toggled
            if(choice == menuChoice)
            {
                chosen.erase(chosen.begin() +menuChoice);
                mvwprintw(selectionMenuWin, choice, width -1, " ");
            }
            else
            {
                chosen.push_back(menuChoice);
                mvwprintw(selectionMenuWin, choice, width -1, "X");
            }
        }
        wgetch(menuWin);*/

        menuHighlight();

        //User made a decision, exit
        if (choice != 0)
            return choice;
    }
}

void Menu::setDrawBorder(bool draw, int bordersWE, int bordersNS)
{
    drawBorder = draw;
    this->bordersWE  = bordersWE;
    this->bordersNS = bordersNS;
}
void Menu::menuHighlight()
{
    int x, y;

    //Start postion of choice lables in window
    x = paddingx;
    y = paddingy;

    //Print options
    for (int i = (page -1) * maxLines; i < (page * maxLines) && i < choices.size(); ++i)
    {
        //Highlight the present choice
        if (highlight == i + 1)
        {
            wattron(menuWin, A_REVERSE);
            mvwprintw(menuWin, y, x, "%s", choices[i].c_str());
            wattroff(menuWin, A_REVERSE);
        }
        else
            mvwprintw(menuWin, y, x, "%s", choices[i].c_str());

        ++y;
    }
    wrefresh(menuWin);
}


ttyProgressDialog::ttyProgressDialog(WINDOW* dialog_win, std::string title, int height, int width, int starty, int startx, std::string message)
{
    dialogWin     = dialog_win;
    this->title   = title;
    this->message = message;
    this->height  = height;
    this->width   = width;
    this->starty  = starty;
    this->startx  = startx;

    drawBorder = true;
    bordersWE  = 0;
    bordersNS  = 0;

    this->dialogWin = dialog_win;
}

void ttyProgressDialog::setDrawBorder(bool draw, int bordersWE, int bordersNS)
{
    drawBorder = draw;
    this->bordersWE  = bordersWE;
    this->bordersNS = bordersNS;
}
bool ttyProgressDialog::Update(double progress, std::string message)
{
    /*Update ETA*/
    //TODO update ETA

    /*Update progress bar*/
    //TODO update progress bar
    mvwprintw(dialogWin, 5, 2, "%f", progress); //print return of downloader progress info
    return true;
}
void ttyProgressDialog::dialog()
{
    /*Create title and message*/
    mvwprintw(dialogWin, 0, 2, title.c_str());
    mvwprintw(dialogWin, 2, 2, message.c_str());

    /*Create ETA*/
    //This will be on the 4th line => index 3
    //TODO add ETA

    /*Create progress bar*/
    //This will be on the 5th line => index 4
    //Generate bar text
    std::string pBar = { " [" };
    int padding = 2;
    for(int i = 0; i < ( width - (int(pBar.length()) + (padding *2) ) ); ++i)
        pBar += " ";
    pBar += "]";

    mvwprintw(dialogWin, 4, 2, pBar.c_str());
}