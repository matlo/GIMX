/*
 * ncursesIO.cpp
 *
 *      Author: Zac
 *     Contact: codeohms@protonmail.com
 *  Created on: 10 Aug. 2018
 */

#include "include/cursesIO.h"


MenuBase::MenuBase(WINDOW* menu_win, int height, int width, int starty, int startx, std::vector<std::string> choices)
{
	this->menuWin = menu_win;
	this->starty  = starty;
	this->startx  = startx;
	this->height  = height;
	this->width   = width;
	scrollok(this->menuWin, true);

	this->choices = choices;
	numChoices = choices.size();
}


Menu::Menu(WINDOW* menu_win, int height, int width, int starty, int startx, std::vector<std::string> choices) \
	: MenuBase(menu_win, height, width, starty, startx, choices)
{
	printLabelVertical = true;
	drawBorder = true;
	bordersWE  = 0;
	bordersNS  = 0;

	this->menuWin = menu_win;
}

int Menu::menuLoop(int startChoice)
{
	int highlight = startChoice;
	if (drawBorder)
		box(menuWin, bordersWE, bordersNS);
	menuHighlight(menuWin, highlight);

	//Calculate 'pages'
	//+1 for bottom border
	int pageSize = (height - (yStart + 1));
	int page = 1;
	enum seekLine { back, next };
	auto turnPage = [&page] (seekOption seek) -> void {
		//Check if we need to turn to next page
		//1st case: turn over last page to first, 2nd case: page up, 3rd case: turn over first page to last, 4th case: page down
		switch(seek)
		{
			case seekOption::next:
				if((highlight +1) > numChoices) //1st
				{
					page = 1;
					break;
				}
				elif((highlight +1) == (page * pageSize) +1 )) //2nd
				{
					++page;
					break;
				}
				//If it gets to this line, something went wrong!

			case seekOption::back:
				if((highlight -1) == 0) //3rd
				{
					page = numChoices / pageSize;
					break;
				}
				elif((highlight -1) == (page -1) * pageSize )) //4th
				{

				}
		}



		if(highlight > numChoices)

		if(page != 1)
		{
			elif(highlight > ((page -1) * pageSize)

		}
		elif(highlight < ((page +1) * pageSize)
	};

	int choice, input;
	while (true)
	{
		choice = 0;
		input = wgetch(menuWin);
		switch (input)
		{
		case KEY_UP:
			turnPage();

			//Gone past first page, head to last
			if (highlight == 1)
				highlight = numChoices;
			else
				--highlight;
			break;
		case KEY_DOWN:
			turnPage(seekOption::next, page);

			//Gone past last page, head to first
			if (highlight == numChoices)
				highlight = 1;
			else
				++highlight;
			break;
		case 10:
			choice = highlight;
			break;
		default:
			break;
		}
		menuHighlight(menuWin, highlight);

		if (choice != 0)
		{
			//User made a decision, exit
			return choice;
		}
	}
}

inline void Menu::setPrintOrientation(bool orientation)
	{ printLabelVertical = orientation; }
void Menu::setDrawBorder(bool draw, int bordersWE, int bordersNS)
{
	drawBorder = draw;
	this->bordersWE  = bordersWE;
	this->bordersNS = bordersNS;
}
void Menu::menuHighlight(WINDOW *menu_win, int highlight, int xStart, int yStart)
{
	int x, y;

	//Start postion of choice lables in window
	x = xStart;
	y = yStart;

	//Print options
	for (int i = 0; i < numChoices; ++i)
	{
		//Highlight the present choice
		if (highlight == i + 1)
		{
			wattron(menu_win, A_REVERSE);
			mvwprintw(menu_win, y, x, "%s", choices[i].c_str());
			wattroff(menu_win, A_REVERSE);
		}
		else
			mvwprintw(menu_win, y, x, "%s", choices[i].c_str());
		
		++y;
	}
	wrefresh(menu_win);
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