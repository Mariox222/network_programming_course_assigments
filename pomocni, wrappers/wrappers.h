int Getaddrinfo(const char *hostname, const char *service, const struct addrinfo* hints, struct addrinfo **result);
int Getnameinfo(const struct sockaddr *sockaddr, socklen_t addrlen, char *host, size_t hostlen, char *serv, size_t servlen, int flags);
int Getsockname(int socket, struct sockaddr *address, socklen_t *address_len);

int Socket(int f, int t, int p);
int Bind(int sfd, const struct sockaddr *addr, int addrlen);
ssize_t Sendto( int sockfd, void *buff, size_t nbytes, int flags, const struct sockaddr* to, socklen_t addrlen);
ssize_t Recvfrom(int sockfd, void *buff, size_t nbytes, int flags, struct sockaddr * from, socklen_t * addrlen);

int Listen(int sockfd, int backlog);
int Connect(int sockfd, const struct sockaddr *server, socklen_t addrlen);
int Accept( int sockfd, struct sockaddr* cliaddr, socklen_t *addrlen);

int Read(int fd, char *buf, int max);
int Write(int fd, char *buf, int num);
ssize_t Send(int s, const void *msg, size_t len, int flags);
ssize_t Recv(int s, void *buf, size_t len, int flags);

int Setsockopt(int sockfd, int level, int optname, const void *opval, socklen_t optlen);

int Pread(int fd, void *buf, size_t nbytes, off_t offset);
int Pwrite(int fd, const void *buf, size_t nbytes, off_t offset);

int Select(int maxfd, fd_set *readset, fd_set *writeset, fd_set *excepset, struct timeval *timeout);
