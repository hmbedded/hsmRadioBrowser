/*
 * menu.c
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
 
#include "countrytable.h"
#include "menu.h"
#include "radiobrowser.h"
 
/* Menus Environment */
typedef struct {
	Menu currentMenu;
	Menu settingsMenu;
	LocationMenu locationMenu;
	StationsMenu stationsMenu;
	Menu parentMenu;
	json_object *countrycode_list_jobj;
	json_object *stations_list_jobj;
	int selection;
} MenuData;
typedef MenuData* MenuEnv;

static MenuData menuData;
static MenuEnv menuEnv = &menuData;

/* Menu Action Function Prototypes*/
static void searchByCountry(void);
static void searchByGenre(void);
static void selectWifi(void);
static void editWifiPassword(void);

/* Fixed Menus */
static MenuItem mainMenu[];
static MenuItem settingsMenu[];

/* Main Menu */
#define MAINMENU_ITEM(x) &mainMenu[x]
static MenuItem mainMenu[] = {
	{ NOTYPE, "Search By Country", searchByCountry, NULL, NULL        , NULL            , MAINMENU_ITEM(1) },
	{ NOTYPE, "Search By Genre"  , searchByGenre  , NULL, NULL        , MAINMENU_ITEM(0), MAINMENU_ITEM(2) },
	{ NOTYPE, "Settings"         , NULL           , NULL, settingsMenu, MAINMENU_ITEM(1), NULL             },
};

/* Settings Menu */
#define SETTINGSMENU_ITEM(x) &settingsMenu[x]
static MenuItem settingsMenu[] = {
	{ NOTYPE, "Select Wifi"       , selectWifi      , NULL    , NULL, NULL                , SETTINGSMENU_ITEM(1) },
	{ NOTYPE, "Edit Wifi Password", editWifiPassword, NULL    , NULL, SETTINGSMENU_ITEM(0), SETTINGSMENU_ITEM(2) },
	{ NOTYPE, "Back"              , NULL            , mainMenu, NULL, SETTINGSMENU_ITEM(1), NULL                 },
}; 

static void addMenuItem(MenuItem *item, MenuType mType, const char *mText, void *mFunc, Menu parent, Menu child, MenuItem *prev, MenuItem *next)
{
	item->mType = mType;
	item->mText = mText;
	item->mFunc = mFunc;
	item->parent = parent;
	item->child = child;
	item->prev = prev;
	item->next = next;
}

static void removeMenu(Menu menu)
{
	MenuItem *item = menu;
	MenuItem *next;

	while (item) {
		next = item->next;
		free(item);
		item = next;
	}
}

static void playRadioStation()
{
	json_object *station_jobj;
	json_object *name_jobj;
	json_object *codec_jobj;
	json_object *bitrate_jobj;

	station_jobj = json_object_array_get_idx(menuEnv->stations_list_jobj, menuEnv->selection-1);
	json_object_object_get_ex(station_jobj, "name", &name_jobj);
	json_object_object_get_ex(station_jobj, "codec", &codec_jobj);
	json_object_object_get_ex(station_jobj, "bitrate", &bitrate_jobj);
	printf("\nStation selected: %d  %s - %s - %dkbps\n", menuEnv->selection, json_object_get_string(name_jobj), json_object_get_string(codec_jobj), json_object_get_int(bitrate_jobj));

	if (menuEnv->parentMenu)
		menuEnv->currentMenu = menuEnv->parentMenu;
	else
		menuEnv->currentMenu = mainMenu;
}

static void buildStationsMenu(char* browserData)
{
	StationsMenuItem *stationListItem = NULL;
	enum json_tokener_error jerr;
	int i;

	/* Clear the current menu */
	if (menuEnv->stationsMenu) {
		removeMenu((Menu)menuEnv->stationsMenu);
		menuEnv->stationsMenu = NULL;
	}

	/* Clear the current station by country list JSON object */
	if (menuEnv->stations_list_jobj) {
		json_object_put(menuEnv->stations_list_jobj);
		menuEnv->stations_list_jobj = NULL;
	}

	/* Parse the returned JSON string */
	menuEnv->stations_list_jobj = json_tokener_parse_verbose(browserData, &jerr);
	if (menuEnv->stations_list_jobj == NULL) {
		fprintf(stderr, "Json Tokener error: %s\n", json_tokener_error_desc(jerr));
	}

	/* Build Stations Menu */
	for (i = 0; i<json_object_array_length(menuEnv->stations_list_jobj); i++) {
		json_object *station_jobj;
		json_object *name_jobj;
		json_object *codec_jobj;
		json_object *bitrate_jobj;
		StationsMenuItem *sbc_item;
		MenuItem *item;
		MenuItem *prev;

		station_jobj = json_object_array_get_idx(menuEnv->stations_list_jobj, i);
		json_object_object_get_ex(station_jobj, "name", &name_jobj);
		json_object_object_get_ex(station_jobj, "codec", &codec_jobj);
		json_object_object_get_ex(station_jobj, "bitrate", &bitrate_jobj);
		sbc_item = malloc(sizeof(StationsMenuItem));
		item = (MenuItem*)sbc_item;
		sbc_item->codec = json_object_get_string(codec_jobj);
		sbc_item->bitrate = json_object_get_int(bitrate_jobj);
		if (stationListItem) {
			stationListItem->menuItem.next = item;
			prev = (MenuItem*)stationListItem;
		}
		else {
			prev = NULL;
			menuEnv->stationsMenu = (StationsMenu)item;
		}
		addMenuItem(item, STATIONSBYCOUNTRY, json_object_get_string(name_jobj), playRadioStation, mainMenu, NULL, prev, NULL);
		stationListItem = sbc_item;
	}
	menuEnv->currentMenu = (Menu)menuEnv->stationsMenu;
}

static void listStationsByCountry()
{
	json_object *countrycode_jobj;
	json_object *name_jobj;
	char *browserData;

	countrycode_jobj = json_object_array_get_idx(menuEnv->countrycode_list_jobj, menuEnv->selection-1);
	json_object_object_get_ex(countrycode_jobj, "name", &name_jobj);
	printf("\ncountrySelected: %d  %s\n", menuEnv->selection, getCountryName(json_object_get_string(name_jobj)));

	/* Get list of countries from radio-browser in JSON format */ 
	browserData = getRadioBrowserData("https://fr1.api.radio-browser.info/json/stations/bycountrycodeexact/gb");

	/* Build Stations Menu */
	buildStationsMenu(browserData);

	/* Free browserData */
	free(browserData);
}

static void searchByCountry()
{
	LocationMenuItem *countryListItem = NULL;
	char *browserData;
	int i;
	enum json_tokener_error jerr;

	/* Clear the current menu */
	if (menuEnv->locationMenu) {
		removeMenu((Menu)menuEnv->locationMenu);
		menuEnv->locationMenu = NULL;
	}

	/* Clear the current country code list JSON object */
	if (menuEnv->countrycode_list_jobj) {
		json_object_put(menuEnv->countrycode_list_jobj);
		menuEnv->countrycode_list_jobj = NULL;
	}

	/* Get list of countries from radio-browser in JSON format */ 
	browserData = getRadioBrowserData("https://fr1.api.radio-browser.info/json/countrycodes");

	/* Parse the returned JSON string */
	menuEnv->countrycode_list_jobj = json_tokener_parse_verbose(browserData, &jerr);
	if (menuEnv->countrycode_list_jobj == NULL) {
		fprintf(stderr, "Json Tokener error: %s\n", json_tokener_error_desc(jerr));
	}

	/* Build Location Menu */
	for (i = 0; i<json_object_array_length(menuEnv->countrycode_list_jobj); i++) {
		json_object *countrycode_jobj;
		json_object *name_jobj;
		LocationMenuItem *loc_item;
		MenuItem *item;
		MenuItem *prev;

		countrycode_jobj = json_object_array_get_idx(menuEnv->countrycode_list_jobj, i);
		json_object_object_get_ex(countrycode_jobj, "name", &name_jobj);
		loc_item = malloc(sizeof(LocationMenuItem));
		item = (MenuItem*)loc_item;
		loc_item->countryCode = json_object_get_string(name_jobj);
		if (countryListItem) {
			countryListItem->menuItem.next = item;
			prev = (MenuItem*)countryListItem;
		}
		else {
			prev = NULL;
			menuEnv->locationMenu = (LocationMenu)item;
		}
		addMenuItem(item, LOCATION, getCountryName(loc_item->countryCode), listStationsByCountry, mainMenu, NULL, prev, NULL);
		countryListItem = loc_item;
	}
	menuEnv->currentMenu = (Menu)menuEnv->locationMenu;

	/* Free browserData */
	free(browserData);
}

static void searchByGenre()
{
	printf("searchByGenre: called\n");
}

static void selectWifi()
{
	printf("selectWifi: called\n");
}

static void editWifiPassword()
{
	printf("editWifiPassword: called\n");
}

void initMenu()
{
	menuEnv->currentMenu = mainMenu;
}

void displayCurrentMenu()
{
	MenuItem *item = menuEnv->currentMenu;
	int i = 1;

	while (item != NULL) {
		printf("%d  %s", i, item->mText);

		switch (item->mType) {
			case STATIONSBYCOUNTRY: {
				StationsMenuItem *sbc_item = (StationsMenuItem*)item;

				printf(" - %s - %dkbps\n", sbc_item->codec, sbc_item->bitrate);
				break;
			}
			case LOCATION:
			case NOTYPE: {
				printf("\n");
			}
		}

		i++;
		item = item->next;
	} 
}

MenuItem *getCurrentMenuItem(int idx)
{
	MenuItem *item = menuEnv->currentMenu;
	int i;

	for (i=1; i<idx; i++) {
		item = item->next;
		if (item == NULL)
			break;
	}

	return item;
}

void selectMenu(Menu menu)
{
	menuEnv->currentMenu = menu;
}

void actionMenuItem(MenuItem *mItem, int idx)
{
	menuEnv->selection = idx;
	menuEnv->parentMenu = mItem->parent;
	mItem->mFunc(menuEnv);
}
