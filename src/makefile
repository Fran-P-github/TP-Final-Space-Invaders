CC := gcc
# Puede ser allegro o rb
PLATFORM ?= allegro
FLAGS := -Wall
OBJS := main.o front_$(PLATFORM).o back.o menu_$(PLATFORM).o

ifeq ($(PLATFORM), allegro)
ALFLAGS := $(shell pkg-config allegro-5 allegro_font-5 allegro_ttf-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5 allegro_image-5 allegro_video-5 --libs --cflags)
DEFINE := ALLEGRO
else
ifeq ($(PLATFORM), rb)
DEFINE := RPI
else
$(error Plataforma no soportada: '$(PLATFORM)' por favor usar: PLATFORM=allegro o PLATFORM=rb)
endif
endif

FLAGS += -DPLATFORM=$(DEFINE)

SpaceInvaders: $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o SpaceInvaders $(ALFLAGS)

main.o: main.c general_defines.h back.h front_$(PLATFORM).h
	$(CC) $(FLAGS) $(ALFLAGS) -c main.c

front_$(PLATFORM).o: front_$(PLATFORM).c front_$(PLATFORM).h general_defines.h back.h
	$(CC) $(FLAGS) $(ALFLAGS) -c front_$(PLATFORM).c

back.o: back.c back.h
	$(CC) $(FLAGS) $(ALFLAGS) -c back.c

menu_$(PLATFORM).o: menu_$(PLATFORM).c menu_$(PLATFORM).h general_defines.h
	$(CC) $(FLAGS) $(ALFLAGS) -c menu_$(PLATFORM).c

clear:
	rm -f *.o SpaceInvaders
