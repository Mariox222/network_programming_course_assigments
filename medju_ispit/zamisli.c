#include "zamisli.h"
#include "pomocni.h"
#include "wrappers.h"

void usage()
{
    printf("Usage: ./zamisli [-t timeout] [port]\n");
    exit(1);
}

void sig_alrm( int sig ) {
	debug_printf("vrijeme isteklo: %d\n", sig);
	exit(1);
}


uint16_t startTCP(int timeout, uint32_t clid, const char * host){
	
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
    error = Getaddrinfo(NULL, "0", &hints, &res);

    if (error)
        errx(1, "%s", gai_strerror(error));

    debug_printf("getaddrinfo finished\n");
    dbgPrintAddrinfo(res);
    
    pid_t pid;
    
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
	
	// get port and addr
	in_port_t port;
	const char * addressStr;
    struct sockaddr_in sin;
    socklen_t addrlen = sizeof(sin);
    
    Getsockname(sockfd, (struct sockaddr *)&sin, &addrlen);
    addressStr = getAddrip4_fromAddr_in((struct sockaddr_in *)res->ai_addr);
    port = ntohs(sin.sin_port);
    
    debug_printf("port is: %d\n", port);
	debug_printf("addr is: %s\n", addressStr);
    
    debug_printf("listening at %s:%d\n", addressStr, port);
    
    freeaddrinfo(res);
    freeaddrinfo(&hints);
    debug_printf("freed addrinfo\n");

    // listen
    Listen(sockfd, 10);

    

    sin_size = sizeof their_addr;
    
    signal(SIGALRM, sig_alrm);
	siginterrupt(SIGALRM, 1);
		
	if ( (pid = fork()) == 0 ){
		alarm(timeout);
	    newfd = Accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		alarm(0);
		
	    debug_printf("accepted\n");
	
	    if ((error = Getnameinfo((struct sockaddr *)&their_addr, sin_size,
	                             hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
	                             NI_NUMERICHOST | NI_NUMERICSERV)))
	    {
	        errx(1, "%s", gai_strerror(error));
	    }
	    debug_printf("got connection from %s:%s\n", hbuf, sbuf);
	    
	    uint32_t nbo_clid = clid;
	    
	    char rStr [34];
		memcpy(rStr, &nbo_clid, 4);
		strcpy(rStr + 4, ":<-FLAG-MrePro-2020-2021-MI->\n");
		
		writen(newfd, rStr, 34);
		
	    close (sockfd);
	    close (newfd);
	    
	    debug_printf("closed sockets\n");
	    exit(0);
	}
	
	return (uint16_t)port;
    
}


int main(int argc, char *argv[])
{

    int tFlag = 0;
    int timeout = 5;
    char port[NI_MAXSERV];
    char ch;
    while ((ch = getopt(argc, argv, "t:")) != -1)
    {
        switch (ch)
        {
        case 't':
            debug_printf("opcija %c: %s\n", ch, optarg);
            timeout = atoi(optarg);
            debug_printf("timeout je %d\n", timeout);
            tFlag = 1;
            break;
        default:
            debug_printf("nepoznata opcija\n");
            usage();
        }
    }

    argc -= optind;
    argv += optind;

    debug_printf("broj preostalih argumenata: %d\n", argc);
    debug_printf("preostali argument: %s\n", argv[0]);

    if (argc == 0)
    {
        strcpy(port, "1234");
    }
    else
    {
        strcpy(port, argv[0]);
    }
    debug_printf("timeout je: %d\n", timeout);
    debug_printf("port je %s\n", port);

    // nas socket i mjesto za addresu posiljatelja
    struct sockaddr_in cli;
    memset(&cli, 0, sizeof(cli));

    // ...
    struct addrinfo hints, *res;
    socklen_t clilen;
    int msglen;
    memset(&hints, 0, sizeof hints);

    // hintovi za getaddrinfo
    hints.ai_family = AF_INET; // AF_INET6, AF_UNSPEC
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // socket za pasivni open, *.port

    // getaddrinfo vraca socket preko kojeg mozemo bindad
    int error;
    error = getaddrinfo(NULL, port, &hints, &res);

    if (error)
        errx(1, "%s", gai_strerror(error));

    debug_printf("getaddrinfo finished\n");
    dbgPrintAddrinfo(res);

    // napravi socket pomocu addrinfo koji smo dobili gore
    int udp_sfd;
    udp_sfd = Socket(res->ai_family,
                 res->ai_socktype,
                 res->ai_protocol);

    // bomd spclet pomocu addrinfo koji smo dobili gore
    Bind(udp_sfd,
         res->ai_addr,
         res->ai_addrlen);

    debug_printf("binded...\n");
    freeaddrinfo(res);
    freeaddrinfo(&hints);
    debug_printf("freed addrinfo\n");
	
	time_t t;
	srand((unsigned) time(&t));
	
	int id_nn_map [100];
	uint32_t next_id = 10;
    char *recievedMsg = malloc(20 * sizeof(char));
    while (1){
		clilen = sizeof(cli);
		// slusaj poruke na portu koji je zadan
		// BLOKIRAJUCE
        debug_printf("\nwaiting for udp...\n");
		msglen = Recvfrom(udp_sfd, recievedMsg, 20, 0, (struct sockaddr *)&cli, &clilen);
		debug_printf("recived something, msglen: %d\n", msglen);
        debug_printf("sender is: %s:%d\n", getAddrip4_fromAddr_in(&cli), get_in_port((struct sockaddr *)&cli));
        
		
        char command [4 + 1];
        for (int i = 0; i < 4; i++){
            command[i] = recievedMsg[i];
        }
        command[4] = 0;

        debug_printf("command is: %s\n", command);

		if (strcmp("INIT", command) == 0){
			debug_printf("got INIT command\n");
			struct INIT *message = (struct INIT*)recievedMsg;
			
            uint16_t max = ntohs(message->max);

            debug_printf("max je: %d\n", max);
            
            uint32_t new_id = next_id++;
			
			int randN = (rand() % (max - 1)) + 1;
			
			id_nn_map[new_id] = randN;
			
			debug_printf("new client id is: %d\n", new_id);
			debug_printf("new randN is: %d\n", randN);
			debug_printf("randN is: %d\n", id_nn_map[new_id]);
			debug_printf("sending id...\n");
			
			char rStr [8];
			strcpy(rStr, "ID");
			rStr[2] = 0;
			rStr[3] = 0;
			memcpy(rStr + sizeof(char) * 4, &new_id, sizeof(new_id));
			
			/*for (int i = 0; i < 8; i++){
				printf("%d\n", rStr[i]);
			}*/
			
			Sendto(udp_sfd, rStr, 8 , 0, (struct sockaddr *)&cli, clilen);
			debug_printf("id sent!\n");
			
		} else if (strcmp("BROJ", command) == 0){
			debug_printf("got BROJ command\n");
			struct BROJ *message = (struct BROJ*)recievedMsg;
			
			uint32_t clid = message->clid;
			uint16_t attemptN = ntohs(message->nn);
			uint16_t quess = ntohs(message->xx);
			
			debug_printf("clid: %d\n", clid);
			debug_printf("attemptN: %d\n", attemptN);
			debug_printf("quess: %d\n", quess); 
			
			debug_printf("number they need to quess: %d\n", id_nn_map[clid]);
			if (quess == id_nn_map[clid]){
				debug_printf("quess correct\n");
				
				char rrStr [12];
				strcpy(rrStr, "OK");
				rrStr[2] = 0;
				rrStr[3] = 0;
				memcpy(rrStr + 4, &clid, sizeof(clid));
				memcpy(rrStr + 8, &message->nn, 2);
				
				uint16_t port = startTCP(timeout, clid, getAddripv4_fromAddrinfo(res));
				debug_printf("port: %d\n", port);
				uint16_t nbo_port = htons(port);
				memcpy(rrStr + 10, &nbo_port, 2);
				
				/*for (int i = 0; i < 12; i++){
					printf("%d: %c   %d\n",i, rrStr[i], rrStr[i]);
				}*/
				
				Sendto(udp_sfd, rrStr, 12 , 0, (struct sockaddr *)&cli, clilen);
				debug_printf("OK sent!\n");
			}
			else if (quess < id_nn_map[clid]){
				debug_printf("number is higher\n");
				
				char rrStr [12];
				strcpy(rrStr, "HI");
				rrStr[2] = 0;
				rrStr[3] = 0;
				memcpy(rrStr + 4, &clid, sizeof(clid));
				memcpy(rrStr + 8, &message->nn, 2);
				rrStr[10] = 0;
				rrStr[11] = 0;
				
				Sendto(udp_sfd, rrStr, 12 , 0, (struct sockaddr *)&cli, clilen);
				debug_printf("HI sent!\n");
				
			} else {
				debug_printf("numer is lower\n");
				
				char rrStr [12];
				strcpy(rrStr, "LO");
				rrStr[2] = 0;
				rrStr[3] = 0;
				memcpy(rrStr + 4, &clid, sizeof(clid));
				memcpy(rrStr + 8, &message->nn, 2);
				rrStr[10] = 0;
				rrStr[11] = 0;
				
				Sendto(udp_sfd, rrStr, 12 , 0, (struct sockaddr *)&cli, clilen);
				debug_printf("LO sent!\n");
			}
		}

	}

    return 0;
}
