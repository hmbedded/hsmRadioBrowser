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
	char *mText;                // Menu text
	void *mFunc;                // Function to call when selected
	void *arg;                  // Argument passed to mFunc
	struct MenuItem *prev;      // Previous menu item (NULL if first)
	struct MenuItem *next;      // Next menu item (NULL if last)
	struct MenuItem *parent;    // Parent menu
} MenuItem;

typedef MenuItem* Menu;

#endif /* _MENU_H */
