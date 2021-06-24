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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "countrytable.h"
#include "getservers.h"
#include "hsmbrowser.h"
#include "menu.h"

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

	/* Initialise menu */
	initMenu();

	/* Main Loop */
	while(!quit) {
		char *input;
		int selection;
		MenuItem *item;

		/* Display Current Menu */
		printf("\n");
		displayCurrentMenu();

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
			item = getCurrentMenuItem(selection);
			if (item->child)
				selectMenu(item->child);
			else if (item->mFunc) {
				actionMenuItem(item, selection);
			}
			else if (item->parent)
				selectMenu(item->parent);
		}
	}

	/* Cleanup Country Table */
	cleanupCountryTable();

	/* Cleanup CURL */
	cleanupCurl();

	return 0;
}
