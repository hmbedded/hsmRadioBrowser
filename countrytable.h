/*
 * countrytable.h
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

#ifndef _COUNTRYTABLE_H
#define _COUNTRYTABLE_H

int initCountryTable(void);
const char* getCountryName(const char *code);
void cleanupCountryTable(void);

#endif /* _COUNTRYTABLE_H */
