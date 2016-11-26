# Define required macros here
SHELL = /bin/sh

IDIR = include/
SDIR = src/
SERVER_ODIR = ${SDIR}server_obj/
CLIENT_ODIR = ${SDIR}client_obj/
LDIR = lib/

SERVER_OBJS = $(addprefix ${SERVER_ODIR}, serverhelp.o networking.o server_main.o)
CLIENT_OBJS = $(addprefix ${CLIENT_ODIR}, clienthelp.o networking.o client_main.o)

CFLAGS = -Wall -g
LDFLAGS=
CC = g++
INCLUDES = -I${IDIR}
LIBS = -lpthread -lcryptopp

SERVER_DRIVER = server
CLIENT_DRIVER = client

${SERVER_ODIR}%.o: ${SDIR}%.cc
	@echo "Compiling server object file..."
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

${CLIENT_ODIR}%.o: ${SDIR}%.cc
	@echo "Compiling client object file..."
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

all: ${SERVER_DRIVER} ${CLIENT_DRIVER}

${SERVER_DRIVER}: ${SERVER_OBJS}
	${CC} ${CFLAGS} ${INCLUDES} -o ${SERVER_DRIVER} ${SERVER_OBJS} ${LIBS} ${LDFLAGS}

${CLIENT_DRIVER}: ${CLIENT_OBJS}
	${CC} ${CFLAGS} ${INCLUDES} -o ${CLIENT_DRIVER} ${CLIENT_OBJS} ${LIBS} ${LDFLAGS}

${SERVER_OBJS}: | ${SERVER_ODIR}

${SERVER_ODIR}:
	mkdir ${SERVER_ODIR}
	@echo "server object directory was created because it did not exist"

${CLIENT_OBJS}: | ${CLIENT_ODIR}

${CLIENT_ODIR}:
	mkdir ${CLIENT_ODIR}
	@echo "client object directory was created because it did not exist"

clean:
	-rm -f ${SERVER_OBJS} ${CLIENT_OBJS} ${SERVER_DRIVER} ${CLIENT_DRIVER}
	@echo "All object files and the executable have been removed"
