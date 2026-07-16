.PHONY: clean

CFLAGS  := -Wall -Werror -g
LD      := gcc
LDFLAGS := ${LDFLAGS} -lrdmacm -libverbs -lpthread

APPS    := client client_modified server

all: ${APPS}

client: common.o client.o
	${LD} -o $@ $^ ${LDFLAGS}

server: common.o server.o
	${LD} -o $@ $^ ${LDFLAGS}

client_modified: common.o client_modified.o
	${LD} -o $@ $^ ${LDFLAGS}

clean:
	rm -f *.o ${APPS}

