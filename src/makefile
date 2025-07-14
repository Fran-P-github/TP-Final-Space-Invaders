ARCH := $(shell uname -m)
ifeq ($(ARCH),armv7l)
    PLATFORM ?= rb
else
    PLATFORM ?= allegro
endif

CC := gcc
FLAGS := -Wall
OBJS := main.o front.o back.o scores.o

ifeq ($(PLATFORM), allegro)
OBJS += menu_allegro.o
ALFLAGS := $(shell pkg-config allegro-5 allegro_font-5 allegro_ttf-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5 allegro_image-5 allegro_video-5 --libs --cflags)
DEFINE := ALLEGRO
else
ifeq ($(PLATFORM), rb)
OBJS += font_3x5.o
RBFLAGS := ../libs/joydisp/disdrv.o ../libs/joydisp/joydrv.o ../libs/SDL2/libAudioSDL2.o -I/usr/local/include -L/usr/local/lib -lSDL2
DEFINE := RPI
else
$(error Plataforma no soportada: '$(PLATFORM)' por favor usar: PLATFORM=allegro o PLATFORM=rb)
endif
endif

FLAGS += -DPLATFORM=$(DEFINE)

SpaceInvaders: $(OBJS) general_defines.h
	$(CC) $(FLAGS) $(OBJS) -o SpaceInvaders $(ALFLAGS) $(RBFLAGS)

main.o: main.c general_defines.h back.h front.h
	$(CC) $(FLAGS) -c main.c

front.o: front_$(PLATFORM).c front.h general_defines.h back.h
	$(CC) $(FLAGS) -c front_$(PLATFORM).c -o front.o

back.o: back.c back.h general_defines.h
	$(CC) $(FLAGS) -c back.c

menu_$(PLATFORM).o: menu_$(PLATFORM).c menu_$(PLATFORM).h general_defines.h
	$(CC) $(FLAGS) -c menu_$(PLATFORM).c

scores.o: scores.h scores.c general_defines.h
	$(CC) $(FLAGS) -c scores.c

# rb only
font_3x5.o: font_3x5.c font_3x5.h
	$(CC) $(FLAGS) -c font_3x5.c

clean:
	rm -f *.o SpaceInvaders
