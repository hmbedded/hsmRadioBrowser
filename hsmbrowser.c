/*
 * hsmbrowser.c
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

#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "countrytable.h"
#include "getservers.h"
#include "hsmbrowser.h"
#include "menu.h"
#include "radiobrowser.h"

static void locationSelected(void *arg)
{
	Env env = arg;
	json_object *countrycode_jobj;
	json_object *name_jobj;

	countrycode_jobj = json_object_array_get_idx(env->countrycode_list_jobj, env->selection-1);
	json_object_object_get_ex(countrycode_jobj, "name", &name_jobj);
	printf("\ncountrySelected: %d  %s\n", env->selection, getCountryName(json_object_get_string(name_jobj)));

	if (env->parentMenu)
		env->currentMenu = env->parentMenu;
	else
		env->currentMenu = env->mainMenu;
}

static void searchByCountry(void *arg)
{
	Env env = arg;
	LocationMenuItem *countryListItem = env->locationMenu;
	char *browserData;
	int i;
	enum json_tokener_error jerr;

	/* Get list of countries from radio-browser in JSON format */ 
	browserData = getRadioBrowserData("https://fr1.api.radio-browser.info/json/countrycodes");

	/* Parse the returned JSON string */
	env->countrycode_list_jobj = json_tokener_parse_verbose(browserData, &jerr);
	if (env->countrycode_list_jobj == NULL) {
		fprintf(stderr, "Json Tokener error: %s\n", json_tokener_error_desc(jerr));
	}

	/* Build Location Menu */
	for (i = 0; i<json_object_array_length(env->countrycode_list_jobj); i++) {
		json_object *countrycode_jobj;
		json_object *name_jobj;
		LocationMenuItem *loc_item;
		MenuItem *item;

		countrycode_jobj = json_object_array_get_idx(env->countrycode_list_jobj, i);
		json_object_object_get_ex(countrycode_jobj, "name", &name_jobj);
		loc_item = malloc(sizeof(LocationMenuItem));
		item = (MenuItem*)loc_item;
		loc_item->countryCode = json_object_get_string(name_jobj);
		item->mText = getCountryName(loc_item->countryCode);
		item->mFunc = locationSelected;
		item->parent = env->mainMenu;
		item->child = NULL;
		item->next = NULL;
		if (countryListItem) {
			countryListItem->menuItem.next = item;
			item->prev = (MenuItem*)countryListItem;
		}
		else {
			item->prev = NULL;
			env->locationMenu = (LocationMenu)item;
		}
		countryListItem = loc_item;
	}

	env->currentMenu = (Menu)env->locationMenu;
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
	env->mainMenu = mainMenu;
	env->settingsMenu = settingsMenu;
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

	/* Intialise app data */
	memset(env, 0, sizeof(AppData));

	/* Initialise the Country Table (iso31660-1) */
	initCountryTable();

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
			else if (item->mFunc) {
				env->selection = selection;
				env->parentMenu = item->parent;
				item->mFunc(env);
			}
			else if (item->parent)
				env->currentMenu = item->parent;
		}
	}

	/* Cleanup Country Table */
	cleanupCountryTable();

	/* Cleanup CURL */
	cleanupCurl();

	return 0;
}