# PLATFORM determined automatically, but can be forced when calling make with PLATFORM=rb or PLATFORM=allegro
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
PLATFORMFLAGS := $(shell pkg-config allegro-5 allegro_font-5 allegro_ttf-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5 allegro_image-5 allegro_video-5 --libs --cflags)
FLAGS += -DPLATFORM=ALLEGRO
else
ifeq ($(PLATFORM), rb)
OBJS += font_3x5.o
PLATFORMFLAGS := ../libs/joydisp/disdrv.o ../libs/joydisp/joydrv.o ../libs/SDL2/libAudioSDL2.o -I/usr/local/include -L/usr/local/lib -lSDL2
FLAGS += -DPLATFORM=RPI
else
$(error Unsupported platform: '$(PLATFORM)'. Please use: PLATFORM=allegro or PLATFORM=rb)
endif
endif

SpaceInvaders: $(OBJS) general_defines.h
	$(CC) $(FLAGS) $(OBJS) -o SpaceInvaders $(PLATFORMFLAGS)

main.o: main.c back.h front.h
	$(CC) $(FLAGS) -c main.c

front.o: front_$(PLATFORM).c front.h general_defines.h back.h
	$(CC) $(FLAGS) -c front_$(PLATFORM).c -o front.o

back.o: back.c back.h general_defines.h
	$(CC) $(FLAGS) -c back.c

scores.o: scores.h scores.c general_defines.h
	$(CC) $(FLAGS) -c scores.c

# allegro only
menu_allegro.o: menu_allegro.c menu_allegro.h general_defines.h
	$(CC) $(FLAGS) -c menu_$(PLATFORM).c

# rb only
font_3x5.o: font_3x5.c font_3x5.h
	$(CC) $(FLAGS) -c font_3x5.c

clean:
	rm -f *.o
