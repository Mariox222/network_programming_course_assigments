#
CFLAGS = -Wall -Wextra -g -DDEBUG
LDFLAGS =
POMOCNI_C = pomocni.c wrappers.c
POMOCNI_O = ${POMOCNI_C:.c=.o}
HEADERS = pomocni.h wrappers.h

PRVI_C = bot.c
PRVI_O = ${PRVI_C:.c=.o}
DRUGI_C = server.c
DRUGI_O = ${DRUGI_C:.c=.o}
all: bot server
bot: $(PRVI_O) $(POMOCNI_O)
	$(CC) $(PRVI_O) $(POMOCNI_O) -o $@
server: $(DRUGI_O) $(POMOCNI_O)
	$(CC) $(DRUGI_O) $(POMOCNI_O) -o $@
bot.o: $(HEADERS)
server.o: $(HEADERS)
$(POMOCNI_O): $(HEADERS)
clean:
	-rm -f bot server *.o *.core
