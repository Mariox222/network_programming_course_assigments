
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "pomocni.h"
#include "wrappers.h"

#define MAXDATASIZE 100
#define ERR_MESSAGE_MAXLEN 50

void usage()
{
    printf("Usage:  ./tcpklijent [-s server] [-p port] [-c] filename\n");
    exit(1);
}

void readAndWriteFile(int sockfd, char *filename, int cFlag, int appending);

int main(int argc, char *argv[])
{
    int pFlag = 0, sFlag = 0, cFlag = 0;
    char port[NI_MAXSERV], server_address[NI_MAXHOST], filename[FILENAME_MAX];
    char ch;
    while ((ch = getopt(argc, argv, "p:s:c")) != -1)
    {
        switch (ch)
        {
        case 'p':
            debug_printf("opcija %c: %s\n", ch, optarg);
            strcpy(port, optarg);
            pFlag = 1;
            break;
        case 's':
            debug_printf("opcija %c: %s\n", ch, optarg);
            strcpy(server_address, optarg);
            sFlag = 1;
            break;
        case 'c':
            debug_printf("opcija %c\n", ch);
            cFlag = 1;
            break;
        default:
            debug_printf("nepoznata opcija\n");
            usage();
        }
    }
    if (pFlag == 0)
        strcpy(port, "1234");
    if (sFlag == 0)
        strcpy(server_address, "127.0.0.1");

    argc -= optind;
    argv += optind;

    if (argc != 1)
    {
        usage();
    }
    strcpy(filename, argv[0]);
    debug_printf("server_address je %s\n", server_address);
    debug_printf("port je %s\n", port);
    debug_printf("filename: %s\n", filename);

    int sockfd;
    struct sockaddr_in their_addr;
    int error;
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags |= AI_CANONNAME;

    if ((error = Getaddrinfo(server_address, port, &hints, &res)))
        errx(1, "%s", gai_strerror(error));

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    debug_printf("created socket\n");

    their_addr.sin_family = AF_INET;
    their_addr.sin_port = ((struct sockaddr_in *)res->ai_addr)->sin_port;
    their_addr.sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    Connect(sockfd, (struct sockaddr *)&their_addr, sizeof their_addr);

    debug_printf("connected...\n");

    off_t offset = 0;
	
	int appending = 0;
    if (cFlag)
    {
		if (access(filename, F_OK) == 0){
			appending = 1;
			offset = fsize(filename) - 1;
		}
		else debug_printf("file nepostoji\n");
    }

    offset = htonl(offset);
    writen(sockfd, &offset, 4);

    //writen(sockfd, &filename, strlen(filename));
    writen(sockfd, &filename, sizeof(filename));

    char code;
    readn(sockfd, &code, 1);
    debug_printf("got code: %c\n", code);
    if (code == 0)
    {
        readAndWriteFile(sockfd, filename, cFlag, appending);
    }
    else
    {
        char err_mess[ERR_MESSAGE_MAXLEN];
        readn(sockfd, err_mess, ERR_MESSAGE_MAXLEN);
        printf("Serverside error: %s\n", err_mess);
        close(sockfd);
        exit(code);
    }

    debug_printf("closing sockets...\n");
    close(sockfd);

    return 0;
}

void readAndWriteFile(int sockfd, char *filename, int cFlag, int appending)
{
    char b;
    int readed, fd;
    off_t position = 0;
    
    int flags = O_WRONLY | O_CREAT;
    
    if (cFlag){
        if (appending) position = fsize(filename) - 1;
    }
    else{
		if (access(filename, F_OK) == 0){
			// file vec postoji
			printf("file already exists\n");
			exit(1);
		}
    }

    if ((fd = open(filename, flags)) < 0)
    {
        printf("Couldn't open file\n");
        return;
    }

    debug_printf("reciving file:\n");
    
    while (1)
    {
        if ((readed = readn(sockfd, &b, 1)) == 0)
        {
            // they sent EOF
            debug_printf("closing file\n");
            close(fd);
            return;
        }
        else
        {
            // pisi datoteku
            Pwrite(fd, &b, 1, position);
            position++;
        }
    }
}
