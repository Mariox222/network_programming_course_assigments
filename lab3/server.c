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
#include <math.h>

#include "pomocni.h"
#include "wrappers.h"

#define MAXLEN 1024
#define STDIN 0
#define BACKLOG 10
#define SI_COM_MAXLEN 6

void usage();

int udp_sfd;
int tcp_sfd;

void closeSock_and_quit()
{
	shutdown(udp_sfd, SHUT_RDWR);
	close(udp_sfd);
	exit(0);
}

void sig_alrm(int sig)
{
	debug_printf("vrijeme isteklo: %d\n", sig);
	return;
}

int main(int argc, char *argv[])
{
	signal(SIGINT, closeSock_and_quit);

	int pFlag = 0, uFlag = 0, tFlag = 0;
	char ch;
	char u_port[4 + 1];
	char t_port[4 + 1];
	char payloadList[MAXLEN];
	char recievedMsg[MAXLEN];
	while ((ch = getopt(argc, argv, "t:u:p:")) != -1)
	{
		switch (ch)
		{
		case 'u':
			debug_printf("opcija %c: %s\n", ch, optarg);
			strcpy(u_port, optarg);
			uFlag = 1;
			break;
		case 't':
			debug_printf("opcija %c: %s\n", ch, optarg);
			strcpy(t_port, optarg);
			tFlag = 1;
			break;
		case 'p':
			debug_printf("opcija %c: %s\n", ch, optarg);
			strcpy(payloadList, optarg);
			debug_printf("payload je %s\n", payloadList);
			pFlag = 1;
			break;
		default:
			debug_printf("nepoznata opcija\n");
			usage();
		}
	}
	if (uFlag == 0)
		strcpy(u_port, "1234");
	if (tFlag == 0)
		strcpy(t_port, "1234");
	if (pFlag == 0)
		strcpy(payloadList, "");

	// nas socket i mjesto za addresu posiljatelja

	struct sockaddr_in cli;
	socklen_t clilen;
	memset(&cli, 0, sizeof(cli));

	// ...
	struct addrinfo hints, *res;

	int error;

	// bindaj udp socket

	// hintovi za getaddrinfo
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // AF_INET6, AF_UNSPEC
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // socket za pasivni open, *.port

	//debug_printf("port: %s\n payload: \"%s\"\n", port, payload);

	
	error = getaddrinfo(NULL, u_port, &hints, &res);

	if (error)
		errx(1, "%s", gai_strerror(error));

	debug_printf("getaddrinfo finished\n");
	dbgPrintAddrinfo(res);

	// napravi socket pomocu addrinfo koji smo dobili gore
	udp_sfd = Socket(res->ai_family,
					 res->ai_socktype,
					 res->ai_protocol);

	// bomd spclet pomocu addrinfo koji smo dobili gore
	Bind(udp_sfd,
		 res->ai_addr,
		 res->ai_addrlen);

	debug_printf("udp binded...\n");

	// bindaj tcp socket

	// hintovi za getaddrinfo
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // AF_INET6, AF_UNSPEC
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // socket za pasivni open, *.port

	//debug_printf("port: %s\n payload: \"%s\"\n", port, payload);

	// getaddrinfo vraca socket preko kojeg mozemo bindad
	error = getaddrinfo(NULL, t_port, &hints, &res);

	if (error)
		errx(1, "%s", gai_strerror(error));

	debug_printf("getaddrinfo finished\n");
	dbgPrintAddrinfo(res);

	// napravi socket pomocu addrinfo koji smo dobili gore
	tcp_sfd = Socket(res->ai_family,
					 res->ai_socktype,
					 res->ai_protocol);

	setReuseAddr(tcp_sfd);
	// bomd spclet pomocu addrinfo koji smo dobili gore
	Bind(tcp_sfd,
		 res->ai_addr,
		 res->ai_addrlen);

	debug_printf("tcp binded...\n");

	Listen(tcp_sfd, BACKLOG);

	// debug ispis gdje slusa sockfd
	listeningAt(tcp_sfd, res);

	freeaddrinfo(res);
	// nikad nemoj freeaddringo &hints
	//freeaddrinfo(&hints);
	debug_printf("freed addrinfo\n");

	// pripremi za select

	// master i temporary fd set
	fd_set master_set;
	fd_set read_fds;
	int fdmax = (tcp_sfd > udp_sfd) ? tcp_sfd : udp_sfd;

	FD_ZERO(&master_set);
	FD_ZERO(&read_fds);

	FD_SET(tcp_sfd, &master_set);
	FD_SET(udp_sfd, &master_set);
	FD_SET(STDIN, &master_set);

	int msglen;
	while (1)
	{
		read_fds = master_set;

		debug_printf("selecting...\n");

		Select(fdmax + 1, &read_fds, NULL, NULL, NULL);

		int i;
		for (i = 0; i <= fdmax; i++)
		{
			if (FD_ISSET(i, &read_fds))
			{
				if (i == STDIN)
				{
					debug_printf("primljeno nesto s STDIN\n");
					char c;
					char buff[1024];
					int j = 0;
					while (read(STDIN, &c, 1) > 0)
					{
						if (c == '\0' || c == '\n')
						{
							break;
						}
						buff[j] = c;
						j++;
					}
					buff[j] = '\0';

					debug_printf("procitano s stdin: %s\n", buff);
					char command[SI_COM_MAXLEN];

					
					for(j = 0; j < (int)strlen(buff); j++)
					{
						//debug_printf("%c\n",buff[j]);
						if (buff[j] == ' ') break;
						command[j] = buff[j];

					}
					command[j] = '\0';

					debug_printf("komanda je %s\n", command);

					if (strcmp("PRINT", command) == 0)
					{
						printf("%s\n", payloadList);
					}
					else if (strcmp("SET", command) == 0)
					{
						j = 4;
						while (buff[j] == ' ')
							j++; // preskoci ako ima vise praznina

						int k = 0;
						while (buff[j] != '\0')
						{
							payloadList[k] = buff[j];
							k++;
							j++;
						}
						payloadList[k] = '\0';
						debug_printf("novi payloadlist je: %s\n", payloadList);
					}
					else if (strcmp("QUIT", command) == 0)
					{
						debug_printf("quiting\n");

						close(tcp_sfd);
						close(udp_sfd);
						debug_printf("closed sockets\n");
						debug_printf("BYEEE!\n");
						exit(0);
					}
				}
				else if (i == tcp_sfd)
				{
					debug_printf("tcp listen socket je citljiv\n");

					struct sockaddr_in their_addr;
					socklen_t sin_size = sizeof(their_addr);
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

					// ne blokirajuci accept
					int newfd = Accept(tcp_sfd, (struct sockaddr *)&their_addr, &sin_size);

					debug_printf("accepted\n");

					// who connected? for debugging purposes only
					if ((error = Getnameinfo((struct sockaddr *)&their_addr, sin_size,
											 hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
											 NI_NUMERICHOST | NI_NUMERICSERV)))
					{
						errx(1, "%s", gai_strerror(error));
					}
					debug_printf("got connection from %s:%s\n", hbuf, sbuf);

					// dodaj novi sfd u popis
					FD_SET(newfd, &master_set);
					if (newfd > fdmax)
						fdmax = newfd; // ažuriraj fdmax

					debug_printf("added new client socket to master fd set\n");
				}
				else if (i == udp_sfd)
				{
					debug_printf("udp socket je citljiv\n");

					clilen = sizeof(cli);
					msglen = Recvfrom(udp_sfd, recievedMsg, MAXLEN, 0, (struct sockaddr *)&cli, &clilen);
					recievedMsg[msglen] = 0; // za printf i usporedbu dolje, ne utjece na sendto()

					debug_printf("recieved %s\n", recievedMsg);

					if (strcmp("HELLO\n", recievedMsg) == 0)
					{

						// concataj payload
						char *stringResult = malloc((sizeof(payloadList) + 10) * sizeof(char) + 1);
						stringResult[0] = '\0';
						strcat(stringResult, "PAYLOAD:");
						strcat(stringResult, payloadList);
						strcat(stringResult, "\n");
						strcat(stringResult, "\0");

						debug_printf("sending with UDP: \"%s\"\n", stringResult);

						// salji onome koji od kojeg smo dobili
						Sendto(udp_sfd, stringResult, strlen(stringResult), 0, (struct sockaddr *)&cli, clilen);

						free(stringResult);
						debug_printf("freed memory");
					}
				}
				else
				{
					// onaj koji se spojio je poslao nesto, socket fd je u varijabli i
					debug_printf("tcp clijent socket is readable\n");

					char buff[1024 + 1];
					int readed = 0;
					buff[sizeof(buff) - 1] = '\0'; // za slucaj kad je readn prekinut alarmom
					
					// je li ovdje potreban alarm? vjerojatno ne
					signal(SIGALRM, sig_alrm);
					siginterrupt(SIGALRM, 1);

					alarm(5);
					readed = Read(i, buff, strlen("HELLO\n"));
					alarm(0);

					buff[readed] = '\0';

					debug_printf("got %s from client\n", buff);
					
					// u slucaju da je klijent poslao EOF, ili nije poslao HELLO\n, prekinut ce se veza
					// u slucaju da je klijent poslao HELLO\n, poslat ce se odgovor i prekinut veza
					if (strcmp("HELLO\n", buff) == 0)
					{
						// concataj payload
						char *stringResult = malloc((sizeof(payloadList) + 10) * sizeof(char) + 1);
						stringResult[0] = '\0';
						strcat(stringResult, "PAYLOAD:");
						strcat(stringResult, payloadList);
						strcat(stringResult, "\n");
						strcat(stringResult, "\0");

						debug_printf("sending with TCP: \"%s\"\n", stringResult);

						// salji
						writen(i, stringResult, strlen(stringResult));

						debug_printf("sent\n");
						
						free(stringResult);
					}
					
					close(i);
					FD_CLR(i, &master_set);

					debug_printf("hung up\n");
					debug_printf("closed newsfd and freed memory\n");
				}
			}
		}
	}
}

void usage()
{
	fprintf(stderr, "%s\n", "Usage:  ./server [-t tcp_port] [-u udp_port] [-p popis]");
	exit(1);
}
