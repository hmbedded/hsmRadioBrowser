/*
 * main.c
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

#include <stdio.h>
#include <unistd.h>

#include "menu.h"
#include "getservers.h"
#include "radiobrowser.h"

typedef enum {
	FALSE,
	TRUE
} bool;

typedef struct {
	RadioBrowserServer serverList;
	Menu currentMenu;
} AppData;

typedef AppData* Env;

static void searchByCountry(void *arg)
{
}

static void searchByGenre(void *arg)
{
}

static void selectWifi(void *arg)
{
}

static void editWifiPassword(void *arg)
{
}

/* Main Menu */
MenuItem mainMenu[] = {
	{ "Search By Country", searchByCountry },
	{ "Search By Genre"  , searchByGenre   },
	{ "Settings"         , NULL            },
	{ NULL }
};

/* Settings Menu */
MenuItem settingsMenu[] = {
	{ "Select Wifi", selectWifi },
	{ "Edit Wifi Password", editWifiPassword },
	{ NULL }
}; 

static void initMenu(Menu menu, Menu parent, Env env)
{
	MenuItem *item = menu;
	int i = 1;

	item[0].prev = NULL;
	item[0].next = &item[1];

	while (item[i].mText != NULL) {
		item[i].prev = &item[i-1];
		item[i].next = &item[i+1];
		item[i].parent = parent;
		item[i].arg = env;
		i++;
	}

	item[i-1].next = NULL;
}

static void initMenus(Env env)
{
	initMenu(mainMenu, NULL, env);
	initMenu(settingsMenu, NULL, env);
}

static void displayMenu(Env env)
{
	MenuItem *item = env->currentMenu;
	int i = 1;

	while (item != NULL) {
		printf("%d  %s\n", i, item->mText);
		i++;
		item = item->next;
	} 
}

int main(void)
{
	AppData appData;
	Env env = &appData;
	_Bool quit = FALSE;

	/* Initialise the CURL library */
	if (initCurl() < 0)
		return -1;

	/* Get servers for the radio browser database */
	getServers("all.api.radio-browser.info");

	/* Initialise static menus */
	initMenus(env);
	env->currentMenu = mainMenu;

	/* Main Loop */
	while(!quit) {
		int ch;

		/* Display Current Menu */
		displayMenu(env);

		/* Wait for user input */
		ch = getchar();
		if (ch == 'q')
			quit = TRUE;
	}

	/* Cleanup CURL */
	cleanupCurl();

	return 0;
}
