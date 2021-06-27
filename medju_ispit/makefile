PROJECT = zamisli
SOURCE = zamisli.c pomocni.c wrappers.c
HEADERS = zamisli.h pomocni.h wrappers.h

#CC = gcc ... ako zelimo kompajler gcc; default je cc
CFLAGS = -Wall -Wextra -g -DDEBUG
LDFLAGS =
OBJECTS = ${SOURCE:.c=.o}

$(PROJECT): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROJECT)

$(OBJECTS): $(HEADERS)

clean:
	-rm -f $(PROJECT) $(OBJECTS) *.core
