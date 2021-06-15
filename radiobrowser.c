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
#include <unistd.h>
#include <curl/curl.h>

//~ size_t callback(void *buffer, size_t size, size_t nmemb, void *userp)
//~ {
	//~ printf("\n\twite_data: size = %ld\n", size);
	//~ write(STDOUT_FILENO, buffer, nmemb);
	//~ printf("\n");

	//~ return nmemb;
//~ }

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

int getRadioBrowserData(char *url, void *callback)
{
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback); 

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK)
		  fprintf(stderr, "curl_easy_perform() failed: %s\n",
				  curl_easy_strerror(res));

		/* always cleanup */
		curl_easy_cleanup(curl);
	}

	return 0;
}
