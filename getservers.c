/*
 * getServers.c
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "getservers.h"

int getServers(char* name)
{
	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[INET6_ADDRSTRLEN];
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;
	
	if ((status = getaddrinfo(name, NULL, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	printf("IP addresses for %s:\n\n", name);
	
	for(p = res;p != NULL; p = p->ai_next) {
		void *addr;
		char *ipver;
		int ret;

		// get the pointer to the address itself,
		// different fields in IPv4 and IPv6:
		if (p->ai_family == AF_INET) { // IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4";
		} else { // IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}

		// convert the IP to a string and print it:
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf("  %s: %s\n", ipver, ipstr);

		// Reverse DNS lookup to get the pretty name
		if (p->ai_family == AF_INET) 
			ret = getnameinfo(p->ai_addr, sizeof(struct sockaddr_in), host, sizeof(host), service, sizeof(service), 0);
		else
			ret = getnameinfo(p->ai_addr, sizeof(struct sockaddr_in6), host, sizeof(host), service, sizeof(service), 0);
		if (ret < 0)
			printf("Error.......%s\n", gai_strerror(ret));
		else
			printf("%d  Pretty Name: %s\n", ret, host);
	}

	freeaddrinfo(res); // free the linked list

	return 0;
}
