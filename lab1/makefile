#
CFLAGS = -Wall -Wextra -g -DDEBUG
LDFLAGS =
POMOCNI_C = pomocni.c
POMOCNI_O = ${POMOCNI_C:.c=.o}
HEADERS = pomocni.h

PRVI_C = bot.c
PRVI_O = ${PRVI_C:.c=.o}
DRUGI_C = UDP_server.c
DRUGI_O = ${DRUGI_C:.c=.o}
all: bot UDP_server
bot: $(PRVI_O) $(POMOCNI_O)
	$(CC) $(PRVI_O) $(POMOCNI_O) -o $@
UDP_server: $(DRUGI_O) $(POMOCNI_O)
	$(CC) $(DRUGI_O) $(POMOCNI_O) -o $@
bot.o: $(HEADERS)
UDP_server.o: $(HEADERS)
$(POMOCNI_O): $(HEADERS)
clean:
	-rm -f bot UDP_server *.o *.core
