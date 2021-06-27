
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

#include "pomocni.h"
#include "wrappers.h"

#define BACKLOG 10
#define ERR_MESSAGE_MAXLEN 50

void usage()
{
    printf("Usage: ./tcpserver [-p port]\n");
    exit(1);
}

void sendFile(int sockfd, char *filename, int offset);

int main(int argc, char *argv[])
{
    int pFlag = 0;
    char port[NI_MAXSERV];
    char ch;
    while ((ch = getopt(argc, argv, "p:")) != -1)
    {
        switch (ch)
        {
        case 'p':
            debug_printf("opcija %c: %s\n", ch, optarg);
            strcpy(port, optarg);
            debug_printf("port je %s\n", port);
            pFlag = 1;
            break;
        default:
            debug_printf("nepoznata opcija\n");
            usage();
        }
    }
    if (pFlag == 0)
        strcpy(port, "1234");

    argc -= optind;
    argv += optind;

    int sockfd, newfd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    socklen_t sin_size;
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);

    // hintovi za getaddrinfo
    hints.ai_family = AF_INET; // AF_INET6, AF_UNSPEC
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // socket za pasivni open, *.port

    // getaddrinfo vraca socket preko kojeg mozemo bindad
    int error;
    error = Getaddrinfo(NULL, NULL, &hints, &res);

    if (error)
        errx(1, "%s", gai_strerror(error));

    debug_printf("getaddrinfo finished\n");
    dbgPrintAddrinfo(res);

    // napravi socket pomocu addrinfo koji smo dobili gore
    sockfd = Socket(res->ai_family,
                    res->ai_socktype,
                    res->ai_protocol);
	
	setReuseAddr(sockfd);
    // bomd spclet pomocu addrinfo koji smo dobili gore
    Bind(sockfd,
         res->ai_addr,
         res->ai_addrlen);

    debug_printf("binded...\n");
    freeaddrinfo(res);
    freeaddrinfo(&hints);
    debug_printf("freed addrinfo\n");

    // listen
    Listen(sockfd, BACKLOG);

    // netocno
    //debug_printf("listening at %s:%s\n", getAddrip4_fromAddr_in(&my_addr), port);

    sin_size = sizeof their_addr;
    newfd = Accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

    debug_printf("accepted\n");

    if ((error = Getnameinfo((struct sockaddr *)&their_addr, sin_size,
                             hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
                             NI_NUMERICHOST | NI_NUMERICSERV)))
    {
        errx(1, "%s", gai_strerror(error));
    }
    debug_printf("got connection from %s:%s\n", hbuf, sbuf);

    int offset;
    readn(newfd, &offset, 4);

    offset = ntohl(offset);

    debug_printf("offset je %d\n", offset);

    char filename[FILENAME_MAX];
    readn(newfd, filename, sizeof filename);

    debug_printf("filename: %s\n", filename);

    sendFile(newfd, filename, offset);
	
	debug_printf("closing sockets...\n");
	
    close(newfd);
    close(sockfd);

    return 0;
}

void sendFile(int sockfd, char *filename, int offset)
{
    int fd;
    char flag = 0;
    if ( (fd = open(filename, O_RDONLY)) < 0 )
    {
        if (errno == ENOENT)
        {
            flag = 1;
            writen(sockfd, &flag, 1);
            writen(sockfd, "file doesn't exist\n", ERR_MESSAGE_MAXLEN);
            debug_printf("file doesn't exist\n");
            return;
        }
        else if (errno == EACCES)
        {
            flag = 2;
            writen(sockfd, &flag, 1);
            writen(sockfd, "permission to read file denied\n", ERR_MESSAGE_MAXLEN);
			debug_printf("permission to read file denied\n");
            return;
        }
        else {
            flag = 3;
            writen(sockfd, &flag, 1);
            writen(sockfd, "some other open() error\n", ERR_MESSAGE_MAXLEN);
            debug_printf("some other open() error\n");
            return;
        }
    }
    writen(sockfd, &flag, 1);

    debug_printf("sending file content:\n");

    off_t position = offset;
    while(1){
        char b;
        int readed;
        readed = Pread(fd, &b, 1, position);
        if (readed == 0){
            // reached EOF
            //printf("\n");
            close(fd);
            return;
        } else {
            //printf("%c", b);
            writen(sockfd, &b, 1);
            position++;
        }

    }

}
