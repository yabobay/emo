#pragma once

#include <stdbool.h>

void image_init();
void image_fini();

struct color { unsigned char r, g, b, a; };

typedef struct image *image;

int load_image(image *img, const char *filename);
void unload_image(image *img);

int image_width(image img);
int image_height(image img);

// returns false if fail
bool get_pixel(image img, int x, int y, struct color *col);
