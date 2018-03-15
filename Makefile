CC=gcc
CFLAGS=-g
LIB=-lasound
SRC=playback.c
EXEC=playback

all:playback

playback: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIB) -o $(EXEC)

run:
	./playback
clean:
	rm -f playback
