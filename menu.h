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

typedef struct MenuItem {
	const char *mText;          // Menu text
	void (*mFunc)(void*);       // Function to call when selected
	struct MenuItem *parent;    // Parent menu
	struct MenuItem *child;     // Child menu
	struct MenuItem *prev;      // Previous menu item (NULL if first)
	struct MenuItem *next;      // Next menu item (NULL if last)
	void *arg;                  // Argument passed to mFunc
} MenuItem;

typedef MenuItem* Menu;

typedef struct LocationMenuItem {
	MenuItem menuItem;
	const char *countryCode;
} LocationMenuItem;

typedef LocationMenuItem* LocationMenu;

#endif /* MENU_H */
