

void debug_printf(char *fmt, ...);
void imp_debug_printf(char *fmt, ...);
void pid_debug_printf(char *fmt, ...);


in_port_t get_in_port(struct sockaddr *sa);
const char* getAddripv4_fromAddrinfo(struct addrinfo *sa);
void printAddrip4_fromAddr_in(struct sockaddr_in *sa);
void getIPandPortToStrings(struct sockaddr *addr, socklen_t sin_size, char *hostBuf, char *portBuf);

void createSockaddr_inFromStrings(struct sockaddr_in * sa, const char * ip_addres, const char* port);
void dbgPrintAddrinfo(struct addrinfo * res);

ssize_t readn(int fd, void *vptr, size_t n);
ssize_t writen(int fd, void *vptr, size_t n);

void setReuseAddr(int sockfd);

off_t fsize(const char *filename);

void listeningAt(int sockfd, struct addrinfo *res);

int compareAddy(struct sockaddr_in* addr1, struct sockaddr_in* addr2);


