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

#define MAX_MESS 100

void usage()
{
    printf("usage: sender [-r sec] [-d delay] [-t | -u] \"string\" ip_adresa port ...\n");
    exit(1);
}
int hadInt = 0;
void sig_alrm(int sig)
{
    hadInt = 1;
    debug_printf("vrijeme isteklo: %d\n", sig);
    return;
}

void tcp_scan(char *message, char *ip_addr, char **ports, int delay, int wait_time, int port_count);
void udp_scan(char *message, char *ip_addr, char **ports, int delay, int wait_time, int port_count);

int main(int argc, char *argv[])
{

    int ch,
        rFlag = 0,
        tFlag = 0,
        uFlag = 0,
        dFlag = 0;
    char ip_addr[NI_MAXHOST];
    char message[MAX_MESS];
    char wait_times[10];
    char delays[10];
    while ((ch = getopt(argc, argv, "r:tud:")) != -1)
    {
        switch (ch)
        {
        case 'r':
            debug_printf("opcija %c: %s\n", ch, optarg);
            strcpy(wait_times, optarg);
            rFlag = 1;
            break;
        case 't':
            tFlag = 1;
            break;
        case 'u':
            uFlag = 1;
            break;
        case 'd':
            debug_printf("opcija %c: %s\n", ch, optarg);
            strcpy(delays, optarg);
            dFlag = 1;
            break;
        default:
            usage();
        }
    }
    argc -= optind;
    argv += optind;

    if (argc < 3)
    {
        usage();
    }
    if (rFlag && dFlag)
    {
        printf("nesmiju -r i -d zajedno\n");
        usage();
    }
    strcpy(message, argv[0]);
    strcpy(ip_addr, argv[1]);

    int wait_time = 0;
    int delay = 0;

    if (rFlag)
    {
        wait_time = atoi(wait_times);
        debug_printf("wait time %d\n", wait_time);
    }
    if (dFlag)
    {
        delay = atoi(delays);
        debug_printf("delay %d\n", delay);
    }

    char *ports[20];
    int port_count = argc - 2;
    for (int i = 0; i < argc - 2; i++)
    {
        ports[i] = argv[i + 2];
        debug_printf("port %d: %s\n", i, ports[i]);
    }
    debug_printf("u = %d, t = %d\n", uFlag, tFlag);
    if ((uFlag && tFlag) || (!(uFlag) && !(tFlag)))
    {
        debug_printf("oba scana\n");
        tcp_scan(message, ip_addr, ports, delay, wait_time, port_count);
        udp_scan(message, ip_addr, ports, delay, wait_time, port_count);
    }
    else if (tFlag)
    {
        tcp_scan(message, ip_addr, ports, delay, wait_time, port_count);
    }
    else if (uFlag)
    {
        udp_scan(message, ip_addr, ports, delay, wait_time, port_count);
    }
    debug_printf("izlaznim normalno\n");
    return 0;
}

void udp_scan(char *message, char *ip_addr, char **ports, int delay, int wait_time, int port_count)
{
    debug_printf(" --- UDP scan\n");
    for (int i = 0; i < port_count; i++)
    {

        char *port = ports[i];
        struct sockaddr_in rec_sa;
        createSockaddr_inFromStrings(&rec_sa, ip_addr, port);

        // napravi socket za rec
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof hints);

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;

        int error = getaddrinfo(NULL, "0", &hints, &res);
        if (error)
            errx(1, "%s", gai_strerror(error));

        debug_printf("getaddrinfo finished\n");
        // dbgPrintAddrinfo(res);

        int sfd = Socket(res->ai_family,
                         res->ai_socktype,
                         res->ai_protocol);

        freeaddrinfo(res);
        // nikad nemoj freeaddringo &hints
        //freeaddrinfo(&hints);
        debug_printf("freed addrinfo\n");

        debug_printf("sending message\n");
        Sendto(sfd, message, strlen(message), 0, (struct sockaddr *)&rec_sa, sizeof(rec_sa));

        debug_printf("sent message\n");

        if (wait_time != 0)
        {
            printf("UDP %s ", port);

            socklen_t addrlen;
            char res_buff[1000];
            int recMesLen;

            signal(SIGALRM, sig_alrm);
            siginterrupt(SIGALRM, 1);

            recMesLen = 0;

            hadInt = 0;

            alarm(wait_time);
            recMesLen = Recvfrom(sfd, res_buff, sizeof(res_buff), 0, (struct sockaddr *)&rec_sa, &addrlen);
            alarm(0);

            res_buff[recMesLen] = 0;

            //debug_printf("Recieved: %s\n", res_buff);

            if (hadInt)
            {
                printf("closed\n");
            }
            else
            {
                printf("open\n");
            }
        }
        if (delay != 0)
        {
            debug_printf("spavaj %d sec\n", delay);
            sleep(delay);
        }
    }
    return;
}

void tcp_scan(char *message, char *ip_addr, char **ports, int delay, int wait_time, int port_count)
{
    debug_printf(" --- TCP scan\n");
    for (int i = 0; i < port_count; i++)
    {

        char *port = ports[i];
        struct sockaddr_in rec_sa;
        createSockaddr_inFromStrings(&rec_sa, ip_addr, port);

        // napravi socket za rec
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof hints);

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        int error = getaddrinfo(NULL, "0", &hints, &res);
        if (error)
            errx(1, "%s", gai_strerror(error));

        debug_printf("getaddrinfo finished\n");
        // dbgPrintAddrinfo(res);

        int sfd = Socket(res->ai_family,
                         res->ai_socktype,
                         res->ai_protocol);

        freeaddrinfo(res);
        // nikad nemoj freeaddringo &hints
        //freeaddrinfo(&hints);
        debug_printf("freed addrinfo\n");

        // connect
        int n;
        if ((n = connect(sfd, (struct sockaddr *)&rec_sa, sizeof(rec_sa))) < 0)
        {
            debug_printf("tcp connect failed\n");
        }
        else
        {
            connect(sfd, (struct sockaddr *)&rec_sa, sizeof(rec_sa));

            debug_printf("connected\n");
            debug_printf("sending message %s\n", message);

            writen(sfd, message, strlen(message));

            debug_printf("sent\n");

            if (wait_time != 0)
            {
                printf("TCP %s ", port);

                debug_printf("waiting for response\n");

                char res_buff[1000];
                int readed = 0;

                signal(SIGALRM, sig_alrm);
                siginterrupt(SIGALRM, 1);

                hadInt = 0;
                alarm(wait_time);
                readed = Read(sfd, res_buff, sizeof(res_buff));
                alarm(0);

                res_buff[readed] = '\0';

                //debug_printf("recieved: %s\n", res_buff);

                if (hadInt)
                {
                    printf("closed\n");
                }
                else
                {
                    printf("open\n");
                }
            }
        }
        close(sfd);
        debug_printf("hangup tcp conn\n");

        if (delay != 0)
        {
            debug_printf("spavaj %d sec\n", delay);
            sleep(delay);
        }
    }
    return;
}
