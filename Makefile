
BD = ./build
SD = ./source
ID = ./include
BN = ./bin
CC = g++
CFLAGS = -std=c++2a -Wall -pedantic -march=native -Ofast

TARGETS = server client

SERVER_OBJECTS = $(BD)/server.o \
 $(BD)/log.o \
 $(BD)/json.o \
 $(BD)/utils.o \
 $(BD)/rand.o \
 $(BD)/tcp.o \
 $(BD)/http.o \
 $(BD)/service.o \
 $(BD)/api.o \
 $(BD)/clock.o \
 $(BD)/trace.o

CLIENT_OBJECTS = $(BD)/test.o \
 $(BD)/log.o \
 $(BD)/json.o \
 $(BD)/utils.o \
 $(BD)/rand.o \
 $(BD)/tcp.o \
 $(BD)/http.o \
 $(BD)/client.o \
 $(BD)/clock.o \
 $(BD)/trace.o

LINKING_SSL = -lssl -lcrypto
LINKING_THREAD = -lpthread

all: $(TARGETS)

server: $(SERVER_OBJECTS) Makefile
	$(CC) $(SERVER_OBJECTS) -o $(BN)/muonbase-server $(LINKING_SSL) $(LINKING_THREAD)

client: $(CLIENT_OBJECTS) Makefile
	$(CC) $(CLIENT_OBJECTS) -o $(BN)/muonbase-client $(LINKING_SSL) $(LINKING_THREAD)

$(BD)/%.o: $(SD)/%.cc
	$(CC) $(CFLAGS) -I$(ID) -I. -o $@ -c $<

clean:
	rm -f *~ $(BD)/* $(SD)/*~ $(ID)/*~ $(BN)/*
