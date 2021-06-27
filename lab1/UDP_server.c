#include <netdb.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "pomocni.h"
#define MAXLEN 512

void usage();

int sfd;

void closeSock(){
	shutdown(sfd, SHUT_RDWR);
	close(sfd);
	exit(0);
}

int main (int argc, char *argv[])
{
	signal(SIGINT, closeSock);
	
	int pFlag = 0, lFlag = 0;
	char ch;
	char port[4 + 1];
	char payload[MAXLEN];
	char recievedMsg[MAXLEN];
	while ((ch = getopt(argc, argv, "l:p:")) != -1 ){
		switch(ch){
			case 'l':
				debug_printf("opcija %c: %s\n", ch, optarg);
				strcpy(port, optarg);
				lFlag = 1;
				break;
			case 'p':
				debug_printf("opcija %c: %s\n", ch, optarg);
				strcpy(payload, optarg);
				debug_printf("payload je %s\n", payload);
				pFlag = 1;
				break;
			default:
				debug_printf("nepoznata opcija\n");
				usage();
		}
	}
	if (lFlag == 0) strcpy(port, "1234");
	if (pFlag == 0) strcpy(payload, "");
	
	// nas socket i mjesto za addresu posiljatelja
	
	struct sockaddr_in cli;
	memset(&cli,0,sizeof(cli));
	
	// ...
	struct addrinfo hints, *res;
	socklen_t clilen;
	int msglen;
	memset(&hints, 0, sizeof hints);
	
	// hintovi za getaddrinfo
	hints.ai_family = AF_INET; // AF_INET6, AF_UNSPEC
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // socket za pasivni open, *.port
	
	debug_printf("port: %s\n payload: \"%s\"\n", port, payload);
	
	// getaddrinfo vraca socket preko kojeg mozemo bindad
	int error;
	error = getaddrinfo(NULL, port, &hints, &res);
	
	if (error) errx(1, "%s", gai_strerror(error));
	
	debug_printf("getaddrinfo finished\n");
	dbgPrintAddrinfo(res);
	
	// napravi socket pomocu addrinfo koji smo dobili gore
	sfd = Socket(res->ai_family,
		res->ai_socktype,
		res->ai_protocol);
	
	// bomd spclet pomocu addrinfo koji smo dobili gore
	Bind(sfd,
		res->ai_addr,
		res->ai_addrlen);
	
	debug_printf("binded...\n");
	freeaddrinfo(res);
	freeaddrinfo(&hints);
	debug_printf("freed addrinfo\n");
	
	while (1){
		clilen = sizeof(cli);
		// slusaj poruke na portu koji je zadan
		// BLOKIRAJUCE
		msglen = Recvfrom(sfd, recievedMsg, MAXLEN, 0, (struct sockaddr *)&cli, &clilen);
		recievedMsg[msglen] = 0; // za printf i usporedbu dolje, ne utjece na sendto()
		
		debug_printf("recieved %s\n", recievedMsg);
		
		// dodaj HELLO\n ako ispitujes s nc
		if (strcmp("HELLO", recievedMsg) == 0){
			debug_printf("on standby...\n");
			
			// concataj payload
			char *stringResult = malloc( (sizeof(payload) + 10) * sizeof( char ) );
			stringResult[0] = '\0';
			strcat(stringResult, "PAYLOAD:");
			strcat(stringResult, payload);
			strcat(stringResult, "\n");
			
			debug_printf("sending \"%s\"\n", stringResult);
			
			// salji onome koji od kojeg smo dobili
			debug_printf("strlen od PAYLOAD:payloadkojijejakodugacakpoglmao je %d\n", strlen("PAYLOAD:payloadkojijejakodugacakpoglmao\n"));
			debug_printf("strlen od stringresult je %d\n", strlen(stringResult));
			Sendto(sfd, stringResult, strlen(stringResult) , 0, (struct sockaddr *)&cli, clilen);
		}
	}
}

void usage(){
	fprintf (stderr,"%s\n", "Usage: ./UDP_server [-l port] [-p payload]");
    exit (1);
}
