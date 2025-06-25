CC := gcc
PLATFORM := allegro

ifeq($(PLATFORM), allegro)
	CFLAGS := -Wall $(shell pkg-config --cflags allegro-5 allegro_font-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5 allegro_image-5)
	LDFLAGS := $(shell pkg-config --libs allegro-5 allegro_font-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5 allegro_image-5)
endif

OBJS := main.o front_$(PLATFORM).o back.o

all: SpaceInvaders

SpaceInvaders: $(OBJS)
		$(CC) $(OBJS) -o SpaceInvaders $(LDFLAGS)

main.o: main.c general_defines.h back.h front_$(PLATFORM).h
		$(CC) $(CFLAGS) -c main.c

front_$(PLATFORM).o: front_$(PLATFORM).c front_$(PLATFORM).h general_defines.h back.h
		$(CC) $(CFLAGS) -c front_$(PLATFORM).c

back.o: back.c back.h
		$(CC) $(CFLAGS) -c back.c

clear:
		rm -f *.o SpaceInvaders

