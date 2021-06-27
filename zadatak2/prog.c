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
#include "prog.h"

void myGetaddrinfo(char *hostName,
                   char *service,
                   int ai_flags,
                   int ai_family,
                   int ai_protocol,
                   struct addrinfo **res);
void usage(void);
void contradictingFlags(void);
void codeError(void);

in_port_t get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
        return (((struct sockaddr_in*)sa)->sin_port);

    return (((struct sockaddr_in6*)sa)->sin6_port);
}

int main(int argc, char *argv[])
{

    int ch,
     rFlag = 0,
     tFlag = 1,
     uFlag = 0,
     xFlag = 0,
     hFlag = 1,
     nFlag = 0,
     ipv4Flag = 1,
     ipv6Flag = 0;
    while ((ch = getopt(argc, argv, "rtuxhn46")) != - 1) {
        switch (ch) {
            case 'r':
                rFlag = 1;
                break;
            case 't':
                tFlag = 1;
                break;
            case 'u':
				tFlag = 0;
                uFlag = 1;
                break;
            case 'x':
                xFlag = 1;
                break;
            case 'h':
                hFlag = 1;
                break;
            case 'n':
				hFlag = 0;
                nFlag = 1;
                break;
            case '4':
                ipv4Flag = 1;
                break;
            case '6':
				ipv4Flag = 0;
                ipv6Flag = 1;
                break;
            case '?':
            default:
                usage();
        }
    }
    argc -= optind;
    argv += optind;
	
	if (argc != 2) {
		usage();
	}
	
	int ai_flags = AI_CANONNAME, ai_family, ai_protocol;
	if (ipv6Flag) {
		ai_family = AF_INET6;
	} else if (ipv4Flag){
		ai_family = AF_INET;
	} else {
		codeError();
	}
	
	if (uFlag) {
		ai_protocol = IPPROTO_UDP;
	} else if (tFlag){
		ai_protocol = IPPROTO_TCP;
	} else {
		codeError();
	}
	
    //printf ("%d, %s\n", argc, argv[0]);
    
    if (rFlag) {
		char* IP_address = argv[0];
		char* port = argv[1];
		
		
	    char host[NI_MAXHOST], sbuf[NI_MAXSERV];
	    int gre;
	    
	    if (ipv6Flag){
		    struct sockaddr_in6 sa6;
		    sa6.sin6_family = AF_INET6;
		    sa6.sin6_port = htons((uint16_t)atoi(port));
		    if (inet_pton(AF_INET6, IP_address, &(sa6.sin6_addr)) != 1)
		    {
		        errx(1, "%s nije valjana IPv6 adresa", IP_address);
		    }
		    gre = getnameinfo((struct sockaddr *)&sa6,
		                      sizeof(struct sockaddr_in6),
		                      host, sizeof(host), sbuf, sizeof(sbuf),
		                      NI_NAMEREQD);
		    if (gre)
		        errx(1, "getnameinfo: %s", gai_strerror(gre));
			printf("%s (%s) %s\n", IP_address, host, sbuf);
		} else {
			struct sockaddr_in sa;
		    sa.sin_family = AF_INET;
			sa.sin_port = htons((uint16_t)atoi(port));
		    if (inet_pton(AF_INET, IP_address, &(sa.sin_addr)) != 1)
		    {
		        errx(1, "%s nije valjana IPv4 adresa", IP_address);
		    }
		    gre = getnameinfo((struct sockaddr *)&sa,
		                      sizeof(struct sockaddr_in),
		                      host, sizeof(host), sbuf, sizeof(sbuf),
		                      NI_NAMEREQD);
		    if (gre)
		        errx(1, "getnameinfo: %s", gai_strerror(gre));
			printf("%s (%s) %s\n", IP_address, host, sbuf);
		}
		
	} else {
		char* hostname = argv[0];
		char* servicename = argv[1];
		
		struct addrinfo *res = NULL, *pamti;
		myGetaddrinfo(hostname,
						servicename,
						ai_flags,
						ai_family,
						ai_protocol,
						&res);
		//printf("IP adresa (CNAME) family socktype proto\n");
	    pamti = res;
	    char addrstr[100];
	    //printf("%s\n", res->ai_canonname);
	    
	    if (ipv6Flag) {
			inet_ntop(
            res->ai_family,
            &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr,
            addrstr, 100);	
		}
	    else {
        inet_ntop(
            res->ai_family,
            &((struct sockaddr_in *)res->ai_addr)->sin_addr,
            addrstr, 100);
        }
        in_port_t printPort = ((struct sockaddr_in*)res->ai_addr)->sin_port;
        if (hFlag){
			   printPort = ntohs(printPort);
		}
        if (xFlag) printf("%s (%s) %04X\n", addrstr,
               res->ai_canonname, printPort);
        else printf("%s (%s) %d\n", addrstr,
               res->ai_canonname, printPort);
        
	    
		freeaddrinfo(pamti); // oslobodi alocirani prostor
	}// %d \n    ntohs(get_in_port(res->ai_addr))
    
    
    return 0;
}

void usage(void) {
    fprintf (stderr,"%s\n", "prog [-r] [-t|-u] [-x] [-h|-n] [-46] {hostname|IP_address} {servicename|port}");
    exit (1);
}

void contradictingFlags(void){
	fprintf (stderr,"%s\n", "a pair of \"choose 1\" flags are both present");
    exit (1);
}
void codeError(void){
	fprintf (stderr,"%s\n", "bug u kodu :(");
    exit (1);
}

void myGetaddrinfo(char *hostName,
                   char *service,
                   int ai_flags,
                   int ai_family,
                   int ai_protocol,
                   struct addrinfo **res)
{
    struct addrinfo hints;
    int error;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags |= ai_flags;
    hints.ai_family = ai_family;
    hints.ai_protocol = ai_protocol;
    error = getaddrinfo(hostName, service, &hints, res);
    if (error) errx(1, "%s", gai_strerror(error));
    //printf("%s\n", res->ai_canonname);
    return ;
}


