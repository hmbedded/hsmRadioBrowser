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
#include <stdlib.h>
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
	printf("searchByCountry: called\n");
}

static void searchByGenre(void *arg)
{
	printf("searchByGenre: called\n");
}

static void selectWifi(void *arg)
{
	printf("selectWifi: called\n");
}

static void editWifiPassword(void *arg)
{
	printf("editWifiPassword: called\n");
}

extern MenuItem mainMenu[];
extern MenuItem settingsMenu[];

/* Main Menu */
MenuItem mainMenu[] = {
	{ "Search By Country", searchByCountry, NULL, NULL         },
	{ "Search By Genre"  , searchByGenre  , NULL, NULL         },
	{ "Settings"         , NULL           , NULL, settingsMenu },
	{ NULL }
};

/* Settings Menu */
MenuItem settingsMenu[] = {
	{ "Select Wifi"       , selectWifi      , NULL, NULL     },
	{ "Edit Wifi Password", editWifiPassword, NULL, NULL     },
	{ "Back"              , NULL            , mainMenu, NULL },
	{ NULL }
}; 

static void initMenu(Menu menu, Env env)
{
	MenuItem *item = menu;
	int i = 1;

	item[0].prev = NULL;
	item[0].next = &item[1];

	while (item[i].mText != NULL) {
		item[i].prev = &item[i-1];
		item[i].next = &item[i+1];
		item[i].arg = env;
		i++;
	}

	item[i-1].next = NULL;
}

static void initMenus(Env env)
{
	initMenu(mainMenu, env);
	initMenu(settingsMenu, env);
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

MenuItem *getMenuItem(Menu menu, int idx)
{
	MenuItem *item = menu;
	int i;

	for (i=1; i<idx; i++) {
		item = item->next;
		if (item == NULL)
			break;
	}

	return item;
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
		char *input;
		int selection;
		MenuItem *item;

		/* Display Current Menu */
		printf("\n");
		displayMenu(env);

		/* Wait for user input */
		printf ("\nSelection: ");
		scanf("%ms", &input);
		printf("\nUser Input: %s\n", input);
		if (input[0] == 'q') {
			quit = TRUE;
			continue;
		}

		/* Action user selection */
		selection = atoi(input);
		free(input);
		if (selection != 0) {
			item = getMenuItem(env->currentMenu, selection);
			if (item->child)
				env->currentMenu = item->child;
			else if (item->mFunc)
				item->mFunc(env);
			else if (item->parent)
				env->currentMenu = item->parent;
		}
	}

	/* Cleanup CURL */
	cleanupCurl();

	return 0;
}
