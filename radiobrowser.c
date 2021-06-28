/*
 * radiobrowser.c
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
#include <curl/curl.h>

#include "hsmbrowser.h"

int initCurl(void)
{
	CURLcode res;

	res = curl_global_init(CURL_GLOBAL_DEFAULT);
	if (res != CURLE_OK) {
		fprintf(stderr, "curl_global_init() failed: %s\n", curl_easy_strerror(res));
		return -1;
	}

	return 0;
}

void cleanupCurl(void)
{
	curl_global_cleanup();
}

static size_t curl_cb(void *buffer, size_t size, size_t nmemb, void *userp)
{
	size_t sizeBytes = size * nmemb;
	size_t outbufsize;
	char **outbuf = userp;

	outbufsize = (*outbuf == NULL) ? 0 : strlen(*outbuf);
	*outbuf = realloc(*outbuf, outbufsize + sizeBytes + 1);
	if (*outbuf == NULL)
		return 0;
	memcpy((*outbuf)+outbufsize, buffer, sizeBytes);
	(*outbuf)[outbufsize + sizeBytes] = 0;

	return sizeBytes;
}


char* getRadioBrowserData(char *url)
{
	CURL *curl;
	CURLcode res;
	char *browserdata = NULL;

	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_cb);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &browserdata);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "HsmRadioBrowser/0.1");

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			return NULL;
		}

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	else {
		fprintf(stderr, "curl_easy_init() return NULL\n");
		return NULL;
	}

	return browserdata;
}
