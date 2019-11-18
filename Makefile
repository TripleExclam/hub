.PHONY: all clean
.DEAFAULT: all

CFLAGS = -g -Wall -pedantic -Werror -std=gnu99
OBJECTS = 2310alice 2310bob 2310hub

all: $(OBJECTS)

2310alice: 2310alice.c
	gcc $(CFLAGS) utilities.c player.c 2310alice.c -o 2310alice

2310bob: 2310bob.c
	gcc $(CFLAGS) utilities.c player.c 2310bob.c -o 2310bob

2310hub: 2310hub.c
	gcc $(CFLAGS) utilities.c 2310hub.c -o 2310hub

clean:
	rm $(OBJECTS)
