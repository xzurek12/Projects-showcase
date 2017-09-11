# Makefile for client/server example
# see 'man make' for more details
SERVER          = server
CLIENT          = mytftpclient
CLIENT_SOURCES  = mytftpclient.c

DEFINES         = 

CFLAGS         = -D_POSIX_SOURCE -std=c99 -pedantic -Wall -g


LIBRARIES       = #-llibrary_name

CC              = gcc
CLIENT_OBJECTS  = $(CLIENT_SOURCES:.c=.o)
INCLUDES        = #-I.
LIBDIRS         = 
LDFLAGS         = $(LIBDIRS) $(LIBRARIES)

###########################################

.SUFFIXES: .c .o

.c.o:
		$(CC) $(CFLAGS) -c $<

###########################################

all:		$(CLIENT)

rebuild:	clean all

$(CLIENT):	$(CLIENT_OBJECTS)
		$(CC) $(CLIENT_OBJECTS) $(LDFLAGS) -o $@

###########################################

clean:
	rm -fr core* *~ $(CLIENT_OBJECTS) $(CLIENT) .make.state .sb
