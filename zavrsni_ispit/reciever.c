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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <arpa/inet.h>


#include "pomocni.h"
#include "wrappers.h"

#define MAX_MESS 100

void usage()
{
    printf("usage: reciever [-i timeout] [-t | -u] \"string\" port ...\n");
    exit(1);
}

int hadInt = 0;
void sig_alrm(int sig)
{
    hadInt = 1;
    debug_printf("vrijeme isteklo: %d\n", sig);
    return;
}

int main(int argc, char *argv[])
{

    int ch,
        iFlag = 0,
        tFlag = 0,
        uFlag = 0;
    char message[MAX_MESS];
    int timeout = 10;
    while ((ch = getopt(argc, argv, "r:tud:")) != -1)
    {
        switch (ch)
        {
        case 'i':
            debug_printf("opcija %c: %s\n", ch, optarg);
            timeout = atoi(optarg);
            iFlag = 1;
            break;
        case 't':
            tFlag = 1;
            break;
        case 'u':
            uFlag = 1;
            break;
        default:
            usage();
        }
    }
    argc -= optind;
    argv += optind;

    if (argc < 2)
    {
        usage();
    }

    strcpy(message, argv[0]);
    debug_printf("message: %s\n", message);

    char *ports[20];
    int port_count = argc - 1;
    for (int i = 0; i < argc - 1; i++)
    {
        ports[i] = argv[i + 1];
        debug_printf("port %d: %s\n", i, ports[i]);
    }

    int udp_fds[20] = {0};
    int tcp_fds[20] = {0};

    // pripremi za select

    // master i temporary fd set
    fd_set master_set;
    fd_set read_fds;
    FD_ZERO(&master_set);
    FD_ZERO(&read_fds);

    if (!(uFlag) && !(tFlag))
    {
        uFlag = 1;
        tFlag = 1;
    }
    if (tFlag)
    {
        int new_sfd;

        for (int i = 0; i < port_count; i++)
        {
            char *port = ports[i];

            // bindaj tcp socket

            // ...
            struct addrinfo hints, *res;

            // hintovi za getaddrinfo
            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_INET; // AF_INET6, AF_UNSPEC
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE; // socket za pasivni open, *.port

            

            // getaddrinfo vraca socket preko kojeg mozemo bindad
            int error = getaddrinfo(NULL, port, &hints, &res);

            if (error)
                errx(1, "%s", gai_strerror(error));

            debug_printf("getaddrinfo finished\n");
            dbgPrintAddrinfo(res);

            // napravi socket pomocu addrinfo koji smo dobili gore
            new_sfd = Socket(res->ai_family,
                             res->ai_socktype,
                             res->ai_protocol);

            setReuseAddr(new_sfd);
            // bomd spclet pomocu addrinfo koji smo dobili gore
            Bind(new_sfd,
                 res->ai_addr,
                 res->ai_addrlen);

            debug_printf("tcp binded...\n");

            Listen(new_sfd, 10);

            // debug ispis gdje slusa sockfd
            listeningAt(new_sfd, res);

            freeaddrinfo(res);
            // nikad nemoj freeaddringo &hints
            //freeaddrinfo(&hints);
            debug_printf("freed addrinfo\n");

            tcp_fds[i] = new_sfd;
        }
    }
    else if (uFlag)
    {
        int new_sfd;

        for (int i = 0; i < port_count; i++)
        {
            char *port = ports[i];

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

            error = getaddrinfo(NULL, port, &hints, &res);

            if (error)
                errx(1, "%s", gai_strerror(error));

            // napravi socket pomocu addrinfo koji smo dobili gore
            new_sfd = Socket(res->ai_family,
                             res->ai_socktype,
                             res->ai_protocol);

            // bomd spclet pomocu addrinfo koji smo dobili gore
            Bind(new_sfd,
                 res->ai_addr,
                 res->ai_addrlen);

            debug_printf("udp binded...\n");

            udp_fds[i] = new_sfd;
        }
    }

    int fdmax = -1;
    // nadji fdmax
    for (int i = 0; i < port_count; i ++){

        FD_SET(tcp_fds[i], &master_set);
        FD_SET(udp_fds[i], &master_set);

        if (fdmax < udp_fds[i]){
            fdmax = udp_fds[i];
        }
        if (fdmax < tcp_fds[i]){
            fdmax = tcp_fds[i];
        }
    }

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
				int isUdp = 0, isTcp = 0;
                for (int j = 0; j < port_count; j ++ ){
                    if (udp_fds[j] == i){
                        isUdp = 1;
                    }
                    if (tcp_fds[j] == i){
                        isTcp = 1;
                    }
                }

                if (isUdp){
                    debug_printf("jedan od udp socketa je citljiv\n");
                }
                else if (isTcp){
                    debug_printf("jedan od tcp socketa je citljiv\n");

                }

            }
        }
    }

    int msglen;

    debug_printf("izlazim normalno\n");
    return 0;
}
