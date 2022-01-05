
BD = ./build
SD = ./source
ID = ./include
BN = ./bin
CC = g++
CFLAGS = -O3 -std=c++2a -Wall -pedantic

TARGETS = database test

DATABASE_OBJECTS = $(BD)/server.o \
 $(BD)/log.o \
 $(BD)/json.o \
 $(BD)/utils.o \
 $(BD)/rand.o \
 $(BD)/tcp.o \
 $(BD)/http.o \
 $(BD)/service.o \
 $(BD)/api.o

TEST_OBJECTS = $(BD)/test.o \
 $(BD)/log.o \
 $(BD)/json.o \
 $(BD)/utils.o \
 $(BD)/rand.o \
 $(BD)/tcp.o \
 $(BD)/http.o \
 $(BD)/client.o

LINKING_SSL = -lssl -lcrypto

all: $(TARGETS)

database: $(DATABASE_OBJECTS) Makefile
	$(CC) $(DATABASE_OBJECTS) -o $(BN)/database.app $(LINKING_SSL)

test: $(TEST_OBJECTS) Makefile
	$(CC) $(TEST_OBJECTS) -o $(BN)/test.app $(LINKING_SSL)

$(BD)/%.o: $(SD)/%.cc
	$(CC) $(CFLAGS) -I$(ID) -I. -o $@ -c $<

clean:
	rm -f *~ $(BD)/* $(SD)/*~ $(ID)/*~ $(BN)/*
