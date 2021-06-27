#include <stdio.h>
#include <stdarg.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <err.h>
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
#include <unistd.h>

#include "pomocni.h"

int Getsockname(int socket, struct sockaddr *address, socklen_t *address_len){
	int n;
	//debug_printf("creating socket...\n");
	if ((n = getsockname(socket, address, address_len)) < 0) {
		err(1, "Getsockname() fatalni error:");
	}
	return(n);
}


int Socket(int f, int t, int p) {
	int n;
	debug_printf("creating socket...\n");
	if ((n = socket(f, t, p)) < 0) {
		err(1, "Socket() fatalni error:");
	}
	return(n);
}

int Bind(int sfd, const struct sockaddr *addr, int addrlen) {
	int n;
	debug_printf("Bind(): binding...\n");
	if ((n = bind(sfd, addr, addrlen)) < 0) {
		err(1, "Bind() fatalni error:");
	}
	return(n);
}

ssize_t Sendto( int sockfd, void *buff, size_t nbytes, int flags,
	const struct sockaddr* to, socklen_t addrlen)
{
	ssize_t n;
	//debug_printf("Sendto(): size: %d\n", nbytes);
	//debug_printf("Sendto(): buff: %s\n", buff);
	
	if ((n = sendto(sockfd, buff, nbytes,flags, to, addrlen)) < 0) {
		err(1, "Sendto() fatalni error n = %d ", n);
	}
	return(n);
}

ssize_t Recvfrom(int sockfd, void *buff, size_t nbytes, int flags, struct sockaddr * from, socklen_t * addrlen){
	ssize_t n;
	if ((n = recvfrom (sockfd, buff, nbytes, flags, from, addrlen)) < 0){
		if (errno != EINTR)
		err(1, "Recvfrom() fatalni error");
	}
	return (n);
}

int Listen(int sockfd, int backlog){
	int n;
	debug_printf("Listen(): ...\n");
	if ((n = listen(sockfd, backlog)) < 0) {
		err(1, "Listen() fatalni error:");
	}
	return(n);
}

int Accept( int sockfd, struct sockaddr* cliaddr, socklen_t *addrlen){
	int n;
	debug_printf("Accept(): ...\n");
	if ((n = accept(sockfd, cliaddr, addrlen)) < 0) {
		err(1, "Accept() fatalni error:");
	}
	return(n);
}

int Connect(int sockfd, const struct sockaddr *server, socklen_t addrlen){
	int n;
	debug_printf("Connect(): ...\n");
	if ((n = connect(sockfd, server, addrlen)) < 0) {
		err(1, "Connect() fatalni error:");
	}
	return(n);
}

int Read(int fd, char *buf, int max){
	int n;
	//debug_printf("Read(): ...\n");
	if ((n = read(fd, buf, max)) < 0) {
		if (errno != EINTR)
		err(1, "Read() fatalni error:");
	}
	return(n);
}

int Write(int fd, char *buf, int num){
	int n;
	//debug_printf("Write(): ...\n");
	if ((n = write(fd, buf, num)) < 0) {
		err(1, "Write() fatalni error:");
	}
	return(n);
}

ssize_t Send(int s, const void *msg, size_t len, int flags) {
	ssize_t n;
	debug_printf("Send() sending: %.*s\n", len, (char*)msg);
	if ((n = send(s, msg, len, flags)) < 0) {
		err(1, "Send() fatalni error:");
	}
	return(n);
}

ssize_t Recv(int s, void *buf, size_t len, int flags) {
    ssize_t n;
	debug_printf("Recv(): ...\n");
	if ((n = recv(s, buf, len, flags)) < 0) {
		err(1, "Recv() fatalni error:");
	}
	return(n);
}

int Getaddrinfo(const char *hostname, const char *service, const struct addrinfo* hints, struct addrinfo **result) {
	int n;
	debug_printf("Getaddrinfo() port: %s host: %s : ...\n", service, hostname);
	if ((n = getaddrinfo(hostname, service, hints, result)) < 0) {
		err(1, "Getaddrinfo() fatalni error:");
	}
	//debug_printf("Getaddrinfo() port: %s host: %s : ...\n", service, hostname);
	return(n);
}

int Setsockopt(int sockfd, int level, int optname, const void *opval, socklen_t optlen) {
	int n;
	debug_printf("Setsockopt(): ...\n");
	if ((n = setsockopt(sockfd, level, optname, opval, optlen)) < 0) {
		err(1, "Setsockopt() fatalni error:");
	}
	return(n);
}

int Getnameinfo(const struct sockaddr *sockaddr, socklen_t addrlen, char *host, size_t hostlen, char *serv, size_t servlen, int flags) {
	int n;
	debug_printf("Getnameinfo(): ...\n");
	if ((n = getnameinfo(sockaddr, addrlen, host, hostlen, serv, servlen, flags)) < 0) {
		err(1, "Getnameinfo() fatalni error:");
	}
	//debug_printf("Getnameinfo done \n");
	return(n);
}

int Pread(int fd, void *buf, size_t nbytes, off_t offset) {
	int n;
	//debug_printf("Pread(): ...\n");
	if ((n = pread(fd, buf, nbytes, offset)) < 0) {
		err(1, "Pread() fatalni error:");
	}
	return(n);
}

int Pwrite(int fd, const void *buf, size_t nbytes, off_t offset) {
	int n;
	//debug_printf("Pwrite(): ...\n");
	if ((n = pwrite(fd, buf, nbytes, offset)) < 0) {
		err(1, "Pwrite() fatalni error:");
	}
	return(n);
}

int Select(int maxfd, fd_set *readset, fd_set *writeset, fd_set *excepset, struct timeval *timeout) {
	int n;
	if ((n = select(maxfd, readset, writeset, excepset, timeout)) < 0){
		err (1, "Select() fatalni error: ");
	}
	return n;
}
