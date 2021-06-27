#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <syslog.h>
#include <stdarg.h>

#include "pomocni.h"
#include "wrappers.h"

struct dataPack {
    short command;
    short blockn;
    char data[512];
};

size_t dpSize(struct dataPack * data){
    return strlen(data->data) + 2 + 2;
}

void childProc(struct sockaddr_in cli, socklen_t clilen, char * data);

void sendError(int sfd, struct sockaddr_in cli, socklen_t clilen, short code, char * message);

void sendBlock(int udp_sfd, struct sockaddr_in cli, socklen_t clilen, char *currentBlock, long blockn);

void sendEmptyPack(int udp_sfd, struct sockaddr_in cli, socklen_t clilen, long blockn);

int waitAck(int udp_sfd, long blockn);