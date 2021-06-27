#ifndef DEBUG
#define DEBUG_PRINTF 0
#else
#define DEBUG_PRINTF 1
#endif

#include <stdio.h>
#include <stdarg.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "wrappers.h"
#include "pomocni.h"

#define DEBUG_ADDR 0

void debug_printf(char *fmt, ...)
{
	va_list args;
	if (DEBUG_PRINTF == 0)
	{
		return;
	}
	else
	{
		va_start(args, fmt);
		fprintf(stderr, "DEBUG: ");
		vfprintf(stderr, fmt, args);
		fflush(NULL);
		va_end(args);
	};
}

void imp_debug_printf(char *fmt, ...)
{
	va_list args;
	if (DEBUG_PRINTF == 0)
	{
		return;
	}
	else
	{
		va_start(args, fmt);
		fprintf(stderr, "DEBUG: --- ");
		vfprintf(stderr, fmt, args);
		fflush(NULL);
		va_end(args);
	};
}

void pid_debug_printf(char *fmt, ...)
{
	va_list args;
	if (DEBUG_PRINTF == 0)
	{
		return;
	}
	else
	{
		va_start(args, fmt);
		fprintf(stderr, "DEBUG: pid %d: ", getpid());
		vfprintf(stderr, fmt, args);
		fflush(NULL);
		va_end(args);
	};
}

in_port_t get_in_port(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
		return (((struct sockaddr_in *)sa)->sin_port);

	return (((struct sockaddr_in6 *)sa)->sin6_port);
}

const char *getAddripv4_fromAddrinfo(struct addrinfo *sa)
{
	static char addrstr[100];
	//inet_ntop(AF_INET, &(sa->sin_addr), dest, INET_ADDRSTRLEN);
	inet_ntop(
		sa->ai_family,
		&((struct sockaddr_in *)sa->ai_addr)->sin_addr,
		addrstr, 100);

	return addrstr;
}

void printAddrip4_fromAddr_in(struct sockaddr_in *sa)
{
	char addrstr[100];
	//inet_ntop(AF_INET, &(sa->sin_addr), dest, INET_ADDRSTRLEN);
	inet_ntop(
		AF_INET,
		&(sa->sin_addr),
		addrstr, 100);
	
	

	printf("%s:%d", addrstr, get_in_port((struct sockaddr *)sa));
}

/*void service_name_to_number (const char* service, char* result){
	char adresa[NI_MAXHOST + NI_MAXSERV];
	char host[NI_MAXHOST];
	char port[NI_MAXSERV];
	struct sockaddr_in sa;
	
	adresa[0] = '\0';
	strcat(adresa, "127.0.0.10");
	//strcat(adresa, service);
	
	sa.sin_family = AF_INET;
	
	if (inet_pton(AF_INET, adresa, &(sa.sin_addr)) != 1) {
		errx(1,"%s nije valjana IPv4 adresa", adresa);
	}
	
	sa.sin_port = htons((uint16_t)atoi(service));
	
	Getnameinfo((struct sockaddr *)&sa, sizeof(struct sockaddr_in), host, sizeof(host), port, sizeof(port), NI_NUMERICSERV);
	
	//debug_printf("%s\n", port);
	strcpy(result, port);
	
	return;
}*/

void createSockaddr_inFromStrings(struct sockaddr_in *sa, const char *ip_addres, const char *port)
{

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // AF_INET6, AF_UNSPEC
	hints.ai_socktype = 0;		 // any socktype
	hints.ai_flags = 0;			 // socket za pasivni open, *.port
	hints.ai_protocol = 0;

	/*for (int i = 0; i < 22; i++){
		printf("%d:  %c   %d\n", i, port[i], port[i]);
	}*/

	Getaddrinfo(ip_addres, port, &hints, &res);

	dbgPrintAddrinfo(res);

	memcpy(sa, (struct sockaddr_in *)res->ai_addr, sizeof(*sa));

	//free(&hints);
	freeaddrinfo(res);
}

void dbgPrintAddrinfo(struct addrinfo *res)
{
	if (DEBUG_ADDR)
	{
		char addrstr[100];

		debug_printf("IP_adresa:port (CNAME) family socktype proto\n");
		while (res)
		{
			inet_ntop(res->ai_family,
					  &((struct sockaddr_in *)res->ai_addr)->sin_addr,
					  addrstr, 100);
			debug_printf("%s:%d (%s) %d %d %d\n", addrstr, ntohs(((struct sockaddr_in *)res->ai_addr)->sin_port),
						 res->ai_canonname,
						 res->ai_family,
						 res->ai_socktype,
						 res->ai_protocol);
			res = res->ai_next;
		}
	}
}

ssize_t readn(int fd, void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nread;
	char *ptr;
	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ((nread = Read(fd, ptr, nleft)) < 0)
		{
			if (errno == EINTR)
				nread = 0; /* and call read() again */
			else
				return (-1);
		}
		else if (nread == 0)
		{
			break; /* EOF */
		}
		nleft -= nread;
		ptr += nread;
	}
	return (n - nleft); /* return >= 0 */
}

ssize_t writen(int fd, void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	char *ptr;
	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ((nwritten = Write(fd, ptr, nleft)) <= 0)
		{
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0; /* and call write again */
			else
				return (-1); /* error */
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return (n);
}

void setReuseAddr(int sockfd)
{
	int on = 1;
	// "address already in use"
	// - ali ako je TIME_WAIT sad ce proci!
	if (Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1)
	{
		err(1, "setsockopt");
	}
}

// char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
// struct sockaddr_in their_addr;
// getIPandPortToStrings ((struct sockaddr *)&their_addr, hbuf, sbuf);
// !!! DOESNT WORK !!!!!
void getIPandPortToStrings(struct sockaddr *addr, socklen_t sin_size, char *hostBuf, char *portBuf)
{
	int error;
	if ((error = Getnameinfo(addr, sin_size,
							 hostBuf, sizeof(hostBuf), portBuf, sizeof(portBuf),
							 NI_NUMERICHOST | NI_NUMERICSERV)))
	{
		errx(1, "%s", gai_strerror(error));
	}
}

off_t fsize(const char *filename)
{
	struct stat st;

	if (stat(filename, &st) == 0)
		return st.st_size;

	return -1;
}

void listeningAt(int sockfd, struct addrinfo *res)
{
	in_port_t port;
	struct sockaddr_in sin;
	socklen_t addrlen = sizeof(sin);

	Getsockname(sockfd, (struct sockaddr *)&sin, &addrlen);

	char addrstr[100];
	//inet_ntop(AF_INET, &(sa->sin_addr), dest, INET_ADDRSTRLEN);
	inet_ntop(
		AF_INET,
		&(((struct sockaddr_in *)res->ai_addr)->sin_addr),
		addrstr, 100);

	port = ntohs(sin.sin_port);

	debug_printf("listening at %s:%d\n", addrstr, port);
}

// samo za ipv4
int compareAddy(struct sockaddr_in *addr1, struct sockaddr_in *addr2)
{
	if (memcmp(&(addr1->sin_port), &(addr2->sin_port), sizeof(addr2->sin_port)) == 0)
	{
		return memcmp(&(addr1->sin_addr), &(addr2->sin_addr), sizeof(addr1->sin_addr));
	}
	else
		return -1;
}
