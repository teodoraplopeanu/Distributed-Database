CC=gcc
CFLAGS=-g -Wall -Wextra

LOAD=load_balancer
SERVER=server
CACHE=lru_cache
UTILS=utils
EXTRA1=dll
EXTRA2=ht
EXTRA3=ll
EXTRA4=queue

# Add new source file names here:
# EXTRA=<extra source file name>

.PHONY: build clean

build: tema2

$(EXTRA1).o: $(EXTRA1).c $(EXTRA1).h
	$(CC) $(CFLAGS) $^ -c

$(EXTRA2).o: $(EXTRA2).c $(EXTRA2).h
	$(CC) $(CFLAGS) $^ -c

$(EXTRA3).o: $(EXTRA3).c $(EXTRA3).h
	$(CC) $(CFLAGS) $^ -c

$(EXTRA4).o: $(EXTRA4).c $(EXTRA4).h
	$(CC) $(CFLAGS) $^ -c

tema2: main.o $(LOAD).o $(SERVER).o $(CACHE).o $(UTILS).o $(EXTRA1).o $(EXTRA2).o $(EXTRA3).o $(EXTRA4).o
	$(CC) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(CACHE).o: $(CACHE).c $(CACHE).h
	$(CC) $(CFLAGS) $^ -c

$(UTILS).o: $(UTILS).c $(UTILS).h
	$(CC) $(CFLAGS) $^ -c

clean:
	rm -f *.o tema2 *.h.gch
