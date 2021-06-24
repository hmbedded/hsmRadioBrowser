/*
 * hsmbrowser.h
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

#ifndef _HSMBROWSER_H
#define _HSMBROWSER_H

#include <json.h>

#include "menu.h"
#include "getservers.h"
#include "radiobrowser.h"

typedef struct {
	RadioBrowserServer serverList;
} AppData;

typedef AppData* Env;

#endif /* HSMBROWSER_H */
