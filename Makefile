CC=gcc
CFLAGS=-g -Wall -pedantic
MAGICK_WAND=`pkg-config --cflags --libs MagickWand`

MAIN=wand.c
MAIN_EXE=wand

SRCMODULES=IHC.c
OBJMODULES=$(SRCMODULES:.c=.o)


$(MAIN_EXE): $(MAIN) $(OBJMODULES)
	$(CC) $(CFLAGS) $^ -o $@ $(MAGICK_WAND)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@ $(MAGICK_WAND)
