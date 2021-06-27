#-DDEBUG
CFLAGS = -Wall -Wextra -g 
LDFLAGS =
POMOCNI_C = pomocni.c wrappers.c
POMOCNI_O = ${POMOCNI_C:.c=.o}
HEADERS = pomocni.h wrappers.h

PRVI_C = tcpklijent.c
PRVI_O = ${PRVI_C:.c=.o}
DRUGI_C = tcpserver.c
DRUGI_O = ${DRUGI_C:.c=.o}
all: tcpklijent tcpserver
tcpklijent: $(PRVI_O) $(POMOCNI_O)
	$(CC) $(PRVI_O) $(POMOCNI_O) -o $@
tcpserver: $(DRUGI_O) $(POMOCNI_O)
	$(CC) $(DRUGI_O) $(POMOCNI_O) -o $@
tcpklijent.o: $(HEADERS)
tcpserver.o: $(HEADERS)
$(POMOCNI_O): $(HEADERS)
clean:
	-rm -f tcpklijent tcpserver *.o *.core
