/*
 * menu.h
 *
 * Author: Howard Mitchell <hm@hmbedded.co.uk> Copyright 2021
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifndef _MENU_H
#define _MENU_H

typedef enum MenuType {
	NOTYPE,
	LOCATION,
	STATIONSBYCOUNTRY
} MenuType;

typedef struct MenuItem {
	MenuType mType;             // Menu type
	const char *mText;          // Menu text
	void (*mFunc)();            // Function to call when selected
	struct MenuItem *parent;    // Parent menu
	struct MenuItem *child;     // Child menu
	struct MenuItem *prev;      // Previous menu item (NULL if first)
	struct MenuItem *next;      // Next menu item (NULL if last)
} MenuItem;

typedef MenuItem* Menu;

typedef struct LocationMenuItem {
	MenuItem menuItem;
	const char *countryCode;
} LocationMenuItem;
typedef LocationMenuItem* LocationMenu;

typedef struct {
	MenuItem menuItem;
	const char *codec;
	int bitrate;
} StationsByCountryMenuItem;
typedef StationsByCountryMenuItem* StationsByCountryMenu;

void initMenu(void);
void displayCurrentMenu(void);
MenuItem *getCurrentMenuItem(int idx);
void selectMenu(Menu menu);
void actionMenuItem(MenuItem *mItem, int idx);

#endif /* MENU_H */
