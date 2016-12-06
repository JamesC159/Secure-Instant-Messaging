# Define required macros here
SHELL = /bin/sh

IDIR = include/
SDIR = src/
OBJDIR = objs/
LDIR = lib/

SERVER_OBJS = $(addprefix ${SDIR}${OBJDIR}, serverhelp.o \
					Buddy.o ClientDB.o BuddyList.o networking.o server_main.o)
CLIENT_OBJS = $(addprefix ${SDIR}${OBJDIR}, clienthelp.o \
					Buddy.o BuddyList.o networking.o client_main.o clientapp.o getch.o)

CFLAGS = -Wall -g -std=c++11
LDFLAGS=
CC = g++
INCLUDES = -I${IDIR}
LIBS = -lpthread -lcrypto++

SERVER_DRIVER = server
CLIENT_DRIVER = client

${SDIR}${OBJDIR}%.o: ${SDIR}%.cc
	@echo "Compiling server object file..."
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@
	
${SDIR}%.cc: ${IDIR}%.h

all: ${SERVER_DRIVER} ${CLIENT_DRIVER}

${SERVER_DRIVER}: ${SERVER_OBJS}
	${CC} ${CFLAGS} ${INCLUDES} -o ${SERVER_DRIVER} ${SERVER_OBJS} ${LIBS} ${LDFLAGS}

${CLIENT_DRIVER}: ${CLIENT_OBJS}
	${CC} ${CFLAGS} ${INCLUDES} -o ${CLIENT_DRIVER} ${CLIENT_OBJS} ${LIBS} ${LDFLAGS}

clean:
	-rm -f ${SDIR}${OBJDIR}*.o ${SERVER_DRIVER} ${CLIENT_DRIVER}
	@echo "All object files and the executable have been removed"
