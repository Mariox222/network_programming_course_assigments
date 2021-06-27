#
CFLAGS = -Wall -Wextra -g -DDEBUG
LDFLAGS =
POMOCNI_C = pomocni.c wrappers.c
POMOCNI_O = ${POMOCNI_C:.c=.o}
HEADERS = pomocni.h wrappers.h

PRVI_C = sender.c
PRVI_O = ${PRVI_C:.c=.o}
DRUGI_C = reciever.c
DRUGI_O = ${DRUGI_C:.c=.o}
all: sender reciever
sender: $(PRVI_O) $(POMOCNI_O)
	$(CC) $(PRVI_O) $(POMOCNI_O) -o $@
reciever: $(DRUGI_O) $(POMOCNI_O)
	$(CC) $(DRUGI_O) $(POMOCNI_O) -o $@
sender.o: $(HEADERS)
reciever.o: $(HEADERS)
$(POMOCNI_O): $(HEADERS)
clean:
	-rm -f sender reciever *.o *.core
