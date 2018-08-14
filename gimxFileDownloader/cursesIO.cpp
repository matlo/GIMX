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

	while (true)
	{
		int choice = 0;
		int input = wgetch(menuWin);
		switch (input)
		{
		case KEY_UP:
			if (highlight == 1)
				highlight = numChoices;
			else
				--highlight;
			break;
		case KEY_DOWN:
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

inline void Menu::printHorizontal(int& x, std::vector<std::string> array, int currentIndex)
	{ x += array[currentIndex].size(); }
inline void Menu::printVertical(int& y)
	{ ++y; }
inline void Menu::setPrintOrientation(bool orientation)
	{ printLabelVertical = orientation; }
void Menu::setDrawBorder(bool draw, int bordersWE, int bordersNS)
{
	drawBorder = draw;
	this->bordersWE  = bordersWE;
	this->bordersNS = bordersNS;
}
void Menu::menuHighlight(WINDOW *menu_win, int highlight, int xLable, int yLable)
{
	int x, y;

	//Postioning of choice lables in window
	x = xLable;
	y = yLable;

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
		//Move to next line, or column, for next label
		if (printLabelVertical)
			printVertical(y);
		else
			printHorizontal(x, choices, i);
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
	std::string pBar = { " []" };
	int padding = 2;
	for(int i = 0; i < ( width - (int(pBar.length()) + (padding *2) ) ); ++i)
		pBar.insert(2, " "); //Insert between square brackets

	mvwprintw(dialogWin, 4, 2, pBar.c_str());
}