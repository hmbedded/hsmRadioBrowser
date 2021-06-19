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
#include <unistd.h>

#include "hsmbrowser.h"
#include "getservers.h"
#include "menu.h"
#include "radiobrowser.h"

char *curlbuffer = NULL;
size_t bufsize = 0;
size_t offset = 0;

static size_t getCountries_cb(void *buffer, size_t size, size_t nmemb, void *userp)
{
	json_object *countries_jobj;
	int i;

	printf("\n\twrite_data: size = %ld, nmemb = %ld\n", size, nmemb);

	bufsize += nmemb;
	curlbuffer = realloc(curlbuffer, bufsize);
	memcpy(curlbuffer+offset, buffer, nmemb);
	offset += nmemb;

	countries_jobj = json_tokener_parse(curlbuffer);
	if (countries_jobj == NULL) {
		printf("Parsing Incomplete\n");
		return nmemb;
	}
	
	free(curlbuffer);
	curlbuffer = NULL;
	bufsize = 0;
	offset = 0;

//	printf("%s\n", json_object_to_json_string(countries_jobj));
	printf("\nObject Type: %s\n", json_type_to_name(json_object_get_type(countries_jobj)));
	printf("Array length: %d\n", json_object_array_length(countries_jobj));
	for (i = 0; i<json_object_array_length(countries_jobj); i++) {
		json_object *country_jobj;
		json_object *name_jobj;

		country_jobj = json_object_array_get_idx(countries_jobj, i);
		json_object_object_get_ex(country_jobj, "name", &name_jobj);
		printf("%s\n", json_object_get_string(name_jobj));
	}

	return nmemb;
}

static void searchByCountry(void *arg)
{
	printf("searchByCountry: called\n");

	getRadioBrowserData("https://fr1.api.radio-browser.info/json/countries", getCountries_cb);
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
