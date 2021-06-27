

#include "tftpserver.h"

#define MAXLEN 516

char ALREADY_ACK[65535 + 2] = {0};
int is_daemon = 0;

void usage()
{
    printf("Usage: ./tftpserver [-d] port_name_or_number\n");
    exit(1);
}

int had_alarm = 0;
void sig_alrm(int sig)
{
    had_alarm = 1;
    sig = sig;
    return;
}

void cleanExit(int exitcode, int fdToClose){
    if (fdToClose > 0){
        close(fdToClose);
    }
    if (is_daemon){
        closelog();
    }
    exit(exitcode);
}

int main(int argc, char *argv[])
{

    signal(SIGALRM, &sig_alrm);
    siginterrupt(SIGALRM, 1);

    int dFlag = 0;
    char port[NI_MAXSERV];
    char ch;
    while ((ch = getopt(argc, argv, "d")) != -1)
    {
        switch (ch)
        {
        case 'd':
            debug_printf("opcija %c\n", ch);
            dFlag = 1;
            break;
        default:
            debug_printf("nepoznata opcija\n");
            usage();
        }
    }
    if (dFlag == 1){
        debug_printf("demon mode is on\n");
        daemon(1, 0);
    }

    argc -= optind;
    argv += optind;

    if (argc != 1)
        usage();

    strcpy(port, argv[argc - 1]);

    debug_printf("port is: %s", port);

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

    debug_printf("getaddrinfo finished\n");
    dbgPrintAddrinfo(res);

    // napravi socket pomocu addrinfo koji smo dobili gore
    int udp_sfd = Socket(res->ai_family,
                         res->ai_socktype,
                         res->ai_protocol);

    // bomd spclet pomocu addrinfo koji smo dobili gore
    Bind(udp_sfd,
         res->ai_addr,
         res->ai_addrlen);

    imp_debug_printf("udp binded...\n");
    listeningAt(udp_sfd, res);

    freeaddrinfo(res);

    debug_printf("addrinfo freed\n");

    while (1)
    {
        imp_debug_printf("waiting for client...\n");

        clilen = sizeof(cli);
        struct dataPack dp;
        Recvfrom(udp_sfd, &dp, MAXLEN, 0, (struct sockaddr *)&cli, &clilen);

        debug_printf("got something\n");

        short command = ntohs(dp.command);

        debug_printf("command is: %d\n", command);
        debug_printf("filename: %s\n", dp.data - 2);
        debug_printf("mode: %s\n", dp.data - 2 + strlen(dp.data - 2) + 1);

        if (command == 1)
        {
            // read req
            imp_debug_printf("got read req\n");

            if (fork() == 0)
            {
                close(udp_sfd);
                childProc(cli, clilen, dp.data);
            }
            else
            {
                continue; // parrent
            }
        }
        else
        {
            sendError(udp_sfd, cli, clilen, 4, "not supported"); // illegal TFTO operation
        }
    }
}

void childProc(struct sockaddr_in cli, socklen_t clilen, char *data)
{
    pid_debug_printf("hello\n");

    char *filename = data - 2;
    char *mode = data - 2 + strlen(data - 2) + 1;

    //pid_debug_printf("my filename is: %s\n", filename);

    int i = 0;
    while (mode[i])
    {
        mode[i] = toupper(mode[i]);
        i++;
    }

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

    error = getaddrinfo(NULL, "0", &hints, &res);

    if (error)
        errx(1, "%s", gai_strerror(error));

    pid_debug_printf("getaddrinfo finished\n");
    dbgPrintAddrinfo(res);

    // napravi socket pomocu addrinfo koji smo dobili gore
    int udp_sfd = Socket(res->ai_family,
                         res->ai_socktype,
                         res->ai_protocol);

    // bomd spclet pomocu addrinfo koji smo dobili gore
    Bind(udp_sfd,
         res->ai_addr,
         res->ai_addrlen);

    imp_debug_printf("udp binded...\n");
    listeningAt(udp_sfd, res);

    freeaddrinfo(res);

    pid_debug_printf("addrinfo freed\n");

    int mode_flag = -1; // ascii = 1, binary = 2

    if (strcmp(mode, "NETASCII") == 0)
    {
        pid_debug_printf("ascii transfer\n");
        mode_flag = 1;
    }
    else if (strcmp(mode, "OCTET") == 0)
    {
        pid_debug_printf("binary transfer\n");
        mode_flag = 2;
    }
    else
    {
        sendError(udp_sfd, cli, clilen, 4, "invalid mode");
    }

    int fd;
    char path[FILENAME_MAX + 10];
    path[0] = '\0';

    char prefix[10 + 1];
    strncpy(prefix, filename, 10);
    prefix[10] = '\0';

    pid_debug_printf("filename prefix is %s\n", prefix);

    if (strcmp(prefix, "/tftpboot/") != 0)
    {
        strcat(path, "/tftpboot/");
    }
    strcat(path, filename);
    
    char addrstr[NI_MAXHOST];
    inet_ntop(
		AF_INET,
		&(cli.sin_addr),
		addrstr, NI_MAXHOST);
    
    printf("%s->%s\n", addrstr, filename);
    
    char ident [200];
    sprintf(ident, "mb5147:MrePro tftpserver[%d]", getpid());
    
    openlog(ident, 0, LOG_FTP);
    
    syslog(LOG_INFO, "%s->%s\n", addrstr, filename);
    

    pid_debug_printf("path is %s\n", path);

    if ((fd = open(path, O_RDONLY)) < 0)
    {
        if (errno == ENOENT)
        {
            sendError(udp_sfd, cli, clilen, 1, "file not found");
        }
        else if (errno == EACCES)
        {
            sendError(udp_sfd, cli, clilen, 2, "permission to access file denied");
        }
        else
        {
            sendError(udp_sfd, cli, clilen, 0, "some other open() error");
        }
    }

    pid_debug_printf("file opened successfully\n");
    pid_debug_printf("sending file starting...\n");

    int readed;
    char c;
    off_t position = 0;
    long blockn = 1;
    char currentBlock[512 + 1];
    i = 0;
    while (1)
    {
        readed = Pread(fd, &c, 1, position);
        if (readed == 0)
        {
            // reached EOF
            close(fd);

            if (i == 512)
            {
                currentBlock[i++] = c;

                pid_debug_printf("sending 1 last full block\n");
                sendBlock(udp_sfd, cli, clilen, currentBlock, blockn++);
                pid_debug_printf("last block %d sent\n", blockn - 1);

                pid_debug_printf("sending empty block\n");
                sendEmptyPack(udp_sfd, cli, clilen, blockn++);
                pid_debug_printf("last block %d sent\n", blockn - 1);
            }
            else
            {
                pid_debug_printf("sending last NOT full block, i: %d\n", i);

                struct dataPack sp;
                memset(&sp, 0, sizeof sp);

                sp.command = htons(3); // data
                sp.blockn = htons(blockn++);

                currentBlock[i] = '\0';

                for (size_t i = 0; i < strlen(currentBlock) + 1; i++)
                {
                    sp.data[i] = currentBlock[i];
                }

                size_t totalSize = dpSize(&sp);

                pid_debug_printf("total packet size: %d\n", totalSize);

                Sendto(udp_sfd, &sp, totalSize, 0, (struct sockaddr *)&cli, clilen);
            }
            pid_debug_printf("done here, bye\n");
            cleanExit(0, udp_sfd);
        }
        else
        {
            if (i >= 512)
            {
                int sent = 0;
                int last_ack = -1;
                while (1)
                {
                    if (sent == 4) break;

                    pid_debug_printf("got 1 full block\n");
                    sendBlock(udp_sfd, cli, clilen, currentBlock, blockn);
                    pid_debug_printf("block %d sent\n", blockn - 1);

                    sent++;

                    last_ack = waitAck(udp_sfd, blockn);

                    if (last_ack == -1){
                        pid_debug_printf("timeout, retransmiting block %d\n", blockn);
                        continue;
                    }

                    if (last_ack == blockn)
                    {
                        ALREADY_ACK[blockn] = 1;
                        pid_debug_printf("got correct ack\n");
                        break;
                    }
                    else if (ALREADY_ACK[last_ack]){
                        pid_debug_printf("double ack %d\n", last_ack);
                        continue;
                    } else {
                        pid_debug_printf("why did this ack arrive? %d\n", last_ack);
                        cleanExit(1, udp_sfd);
                    }
                }

                if (sent == 4)
                {
                    printf("proccess %d connection timeout\n", getpid());
                    cleanExit(0, udp_sfd);
                }
                i = 0;

                blockn++;
                continue;
            }
            else
            {
                if (c == '\n' && mode_flag == 1)
                { // ako je mode asci
                    currentBlock[i++] = '\r';
                }
                currentBlock[i++] = c;
                position++;
                continue;
            }
        }
    }

    cleanExit(0, udp_sfd);
}

void sendError(int sfd, struct sockaddr_in cli, socklen_t clilen, short code, char *message)
{
    struct dataPack sp;
    memset(&sp, 0, sizeof sp);

    sp.command = htons(5);
    sp.blockn = code;
    strcpy(sp.data, message);

    size_t totalSize = dpSize(&sp);

    pid_debug_printf("sending error message: %s\n", message);
    pid_debug_printf("sp.data: %s (%d)\n", sp.data, strlen(sp.data));
    pid_debug_printf("total packet size: %d\n", totalSize);

    Sendto(sfd, &sp, totalSize + 1, 0, (struct sockaddr *)&cli, clilen);

    fprintf(stderr, "TFTP ERROR %d %s\n", code, message);
    syslog(LOG_INFO, "TFTP ERROR %d %s\n", code, message);

    cleanExit(1, sfd);
}

void sendBlock(int udp_sfd, struct sockaddr_in cli, socklen_t clilen, char *currentBlock, long blockn)
{
    struct dataPack sp;
    memset(&sp, 0, sizeof sp);

    sp.command = htons(3); // data
    sp.blockn = htons(blockn);
    sp.data[0] = '\0';

    for (int i = 0; i < 512; i++)
    {
        sp.data[i] = currentBlock[i];
    }

    Sendto(udp_sfd, &sp, sizeof sp, 0, (struct sockaddr *)&cli, clilen);
}

void sendEmptyPack(int udp_sfd, struct sockaddr_in cli, socklen_t clilen, long blockn)
{

    pid_debug_printf("sending empty package to signal EOF\n");

    struct dataPack sp;
    memset(&sp, 0, sizeof sp);

    sp.command = htons(3); // data
    sp.blockn = htons(blockn);
    sp.data[0] = '\0'; // nece se slati data

    size_t totalSize = dpSize(&sp);

    debug_printf("total packet size: %d\n", totalSize);

    Sendto(udp_sfd, &sp, 4, 0, (struct sockaddr *)&cli, clilen);
}

int waitAck(int udp_sfd, long blockn)
{
    pid_debug_printf("waiting for ack for %d\n", blockn);

    struct sockaddr_in cli;
    socklen_t clilen;
    memset(&cli, 0, sizeof(cli));

    clilen = sizeof(cli);
    struct dataPack dp;

    alarm(1);
    Recvfrom(udp_sfd, &dp, sizeof dp, 0, (struct sockaddr*)&cli, &clilen);
    alarm(0);

    short command = ntohs(dp.command);
    short ack_blockn = ntohs(dp.blockn);

    pid_debug_printf("command %d\n", command);
    pid_debug_printf("ack_blockn %d\n", ack_blockn);

    if (had_alarm)
    {
        had_alarm = 0;
        return -1;
    }

    if (command == 4)
    {

        return blockn;
    }
    else
    {
        sendError(udp_sfd, cli, clilen, 4, "expected ack");
    }
    return -666;
}
