all: emo

MAGICK_CFLAGS=$(shell pkg-config MagickCore --cflags)
MAGICK_LDFLAGS=$(shell pkg-config MagickCore --libs)

emo: emo.o image.o
	gcc emo.o image.o -o emo $(MAGICK_LDFLAGS) $(CFLAGS)

emo.o: emo.c image.h
	gcc -c emo.c $(MAGICK_CFLAGS) $(CFLAGS)

image.o: image_magickcore.c image.h
	gcc -c image_magickcore.c -o image.o $(MAGICK_CFLAGS) $(CFLAGS)

clean:
	rm -f *.o emo
