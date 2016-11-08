# Define required macros here
SHELL = /bin/sh

IDIR = include/
SDIR = src/
ODIR = ${SDIR}obj/
LDIR = lib/

OBJS = $(addprefix ${ODIR}, Server.o Client.o Socket.o main.o)
CFLAGS = -Wall -g
LDFLAGS=
CC = g++
INCLUDES = -I ${IDIR}
LIBS = -lpthread

MAINFUNC = ${SDIR}main.cc
EXEC = driver

${ODIR}%.o: ${SDIR}%.cc
	@echo "Compiling object file..."
	${CC} ${CFLAGS} ${INCLUDES} -c $< -o $@

all: ${OBJS}
	@echo "Compiling project together..."
	${CC} ${CFLAGS} ${INCLUDES} -o ${EXEC} ${OBJS} ${LIBS} ${LDFLAGS}
	
	
clean:
	-rm -f ${OBJS} ${EXEC}
	@echo "All object files and the executable have been removed"
	
${OBJS}: | ${ODIR}

${ODIR}:
	mkdir ${ODIR}
	@echo "${ODIR} was created because it did not exist"
