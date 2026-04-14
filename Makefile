all: emo

MAGICK_CFLAGS=$(shell MagickWand-config --cflags)
MAGICK_LDFLAGS=$(shell MagickWand-config --libs --ldflags)

MAGICK_CFLAGS=$(shell MagickCore-config --cflags)
MAGICK_LDFLAGS=$(shell MagickCore-config --libs --ldflags)

emo: emo.o image.o
	gcc emo.o image.o -o emo $(MAGICK_LDFLAGS) $(CFLAGS)

emo.o: emo.c image.h
	gcc -c emo.c $(MAGICK_CFLAGS) $(CFLAGS)

image.o: image_magickcore.c image.h
	gcc -c image_magickcore.c -o image.o $(MAGICK_CFLAGS) $(CFLAGS)

clean:
	rm -f *.o emo
