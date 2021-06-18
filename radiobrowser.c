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
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <json.h>

char *curlbuffer = NULL;
size_t bufsize = 0;
size_t offset = 0;

size_t callback(void *buffer, size_t size, size_t nmemb, void *userp)
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

int getRadioBrowserData(char *url)
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
