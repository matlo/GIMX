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
	height = 10 + paddingy +1;
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
			else if(highlight == page * pageSize) //2nd
			{
				++page;
				draw();
				break;
			}

			break;

		case seekOption::back:
			if((highlight -1) == 0) //3rd
			{
				//+0.5 for rounding to next integer 
				page = ceil( float(numChoices) / float(pageSize) );
				draw();
				break;
			}
			else if(highlight -1 == (page -1) * pageSize) //4th
			{
				--page;
				draw();
				break;
			}

			break;
	}
}
int Menu::menuLoop(int startChoice)
{
	highlight = startChoice;

	//-1 so 0 initialised
	pageSize = (height - (paddingy)) -1;
	page = 1;

/*	for(std::string name : choices)
	{
		if(name > (width - (xpadding *2)))
		{
			
		}
	}*/

	draw();
	menuHighlight();

	int choice, input;
	while (true)
	{
		choice = 0;
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
		case 10:
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

		if (choice != 0)
		{
			//User made a decision, exit
			return choice;
		}
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
	for (int i = (page -1) * pageSize; i < (page * pageSize) && i < choices.size(); ++i)
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