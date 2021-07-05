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
static void handlePreset(void);
static void turnRadioOff(void);
static void selectWifiNetwork(void);
static void editWifiPassword(void);
static void selectTimezone(void);
static void showFirmwareVersion(void);
static void upgradeFirmware(void);

/* Fixed Menus */
static MenuItem mainMenu[];
static MenuItem settingsMenu[];

/* Main Menu */
#define MAINMENU_ITEM(x) &mainMenu[x]
static MenuItem mainMenu[] = {
	{ NOTYPE, "Location" , searchByCountry, NULL, NULL        , NULL             , MAINMENU_ITEM(1)  },  // Main Menu Item 0
	{ NOTYPE, "Genre"    , searchByGenre  , NULL, NULL        , MAINMENU_ITEM(0) , MAINMENU_ITEM(2)  },  // Main Menu Item 1
	{ NOTYPE, "Preset 1" , handlePreset   , NULL, NULL        , MAINMENU_ITEM(1) , MAINMENU_ITEM(3)  },  // Main Menu Item 2
	{ NOTYPE, "Preset 2" , handlePreset   , NULL, NULL        , MAINMENU_ITEM(2) , MAINMENU_ITEM(4)  },  // Main Menu Item 3
	{ NOTYPE, "Preset 3" , handlePreset   , NULL, NULL        , MAINMENU_ITEM(3) , MAINMENU_ITEM(5)  },  // Main Menu Item 4
	{ NOTYPE, "Preset 4" , handlePreset   , NULL, NULL        , MAINMENU_ITEM(4) , MAINMENU_ITEM(6)  },  // Main Menu Item 5
	{ NOTYPE, "Preset 5" , handlePreset   , NULL, NULL        , MAINMENU_ITEM(5) , MAINMENU_ITEM(7)  },  // Main Menu Item 6
	{ NOTYPE, "Preset 6" , handlePreset   , NULL, NULL        , MAINMENU_ITEM(6) , MAINMENU_ITEM(8)  },  // Main Menu Item 7
	{ NOTYPE, "Preset 7" , handlePreset   , NULL, NULL        , MAINMENU_ITEM(7) , MAINMENU_ITEM(9)  },  // Main Menu Item 8
	{ NOTYPE, "Preset 8" , handlePreset   , NULL, NULL        , MAINMENU_ITEM(8) , MAINMENU_ITEM(10) },  // Main Menu Item 9
	{ NOTYPE, "Preset 9" , handlePreset   , NULL, NULL        , MAINMENU_ITEM(9) , MAINMENU_ITEM(11) },  // Main Menu Item 10
	{ NOTYPE, "Preset 10", handlePreset   , NULL, NULL        , MAINMENU_ITEM(10), MAINMENU_ITEM(12) },  // Main Menu Item 11
	{ NOTYPE, "Preset 11", handlePreset   , NULL, NULL        , MAINMENU_ITEM(11), MAINMENU_ITEM(13) },  // Main Menu Item 12
	{ NOTYPE, "Preset 12", handlePreset   , NULL, NULL        , MAINMENU_ITEM(12), MAINMENU_ITEM(14) },  // Main Menu Item 13
	{ NOTYPE, "Settings" , NULL           , NULL, settingsMenu, MAINMENU_ITEM(13), MAINMENU_ITEM(15) },  // Main Menu Item 14
	{ NOTYPE, "Off"      , turnRadioOff   , NULL, NULL        , MAINMENU_ITEM(14), NULL              },  // Main Menu Item 15
};

/* Settings Menu */
#define SETTINGSMENU_ITEM(x) &settingsMenu[x]
static MenuItem settingsMenu[] = {
	{ NOTYPE, "Scan Wifi"       , selectWifiNetwork   , NULL    , NULL, NULL                , SETTINGSMENU_ITEM(1) },  // Settings Menu Item 0
	{ NOTYPE, "Wifi Password"   , editWifiPassword    , NULL    , NULL, SETTINGSMENU_ITEM(0), SETTINGSMENU_ITEM(2) },  // Settings Menu Item 1
	{ NOTYPE, "Timezone"        , selectTimezone      , NULL    , NULL, SETTINGSMENU_ITEM(1), SETTINGSMENU_ITEM(3) },  // Settings Menu Item 2
	{ NOTYPE, "Version"         , showFirmwareVersion , NULL    , NULL, SETTINGSMENU_ITEM(2), SETTINGSMENU_ITEM(4) },  // Settings Menu Item 3
	{ NOTYPE, "Firmware Upgrade", upgradeFirmware     , NULL    , NULL, SETTINGSMENU_ITEM(3), SETTINGSMENU_ITEM(5) },  // Settings Menu Item 4
	{ NOTYPE, "Back"            , NULL                , mainMenu, NULL, SETTINGSMENU_ITEM(4), NULL                 },  // Settings Menu Item 5
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

static void searchByGenre(void)
{
	printf("searchByGenre: called\n");
}

static void handlePreset(void)
{
	printf("handlePreset: called\n");
}

static void turnRadioOff(void)
{
	printf("turnRadioOff: called\n");
}

static void selectWifiNetwork(void)
{
	printf("selectWifiNetwork: called\n");
}

static void editWifiPassword(void)
{
	printf("editWifiPassword: called\n");
}

static void selectTimezone(void)
{
	printf("selectTimezone: called\n");
}

static void showFirmwareVersion(void)
{
	printf("showFirmwareVersion: called\n");
}

static void upgradeFirmware(void)
{
	printf("upgradeFirmware: called\n");
}

void initMenu(void)
{
	menuEnv->currentMenu = mainMenu;
}

void displayCurrentMenu(void)
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
