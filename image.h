#pragma once

#include <stdbool.h>
#include <stdio.h>

void image_init();
void image_fini();

struct color { unsigned char r, g, b, a; };

typedef struct image *image;

typedef enum IMAGE_ERROR { COOL, THAT_WASNT_AN_IMAGE } IMAGE_ERROR;

IMAGE_ERROR load_image_from_filename(image *img, const char *filename);
IMAGE_ERROR load_image_from_file_handle(image *img, FILE *fh);
void unload_image(image *img);

int image_width(image img);
int image_height(image img);

// returns false if fail
bool get_pixel(image img, int x, int y, struct color *col);
