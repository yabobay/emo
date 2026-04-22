#include "image.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <MagickCore/MagickCore.h>

typedef struct image {
    ImageInfo *img_info;
    Image *img;
} *image;

ExceptionInfo *e;

void image_init() {
    // TODO: actually check this thing
    e = AcquireExceptionInfo();
}

void image_fini() {}

int load_image_from_filename(image *img, const char *filename) {
    *img = malloc(sizeof(struct image));
    (*img)->img_info = CloneImageInfo((ImageInfo*) NULL);
    strcpy((*img)->img_info->filename, filename);
    (*img)->img = ReadImage((*img)->img_info, e);
    CatchException(e);
    return 0;
}

// TODO: what does CatchException do

int load_image_from_file_handle(image *img, FILE *fh) {
    assert(fh);
    fseek(fh, 0L, SEEK_END);
    size_t size = ftell(fh);
    rewind(fh);
    char *blob = malloc(size);
    fread(blob, size, 1, fh);

    *img = malloc(sizeof(struct image));
    (*img)->img_info = CloneImageInfo((ImageInfo*) NULL);
    (*img)->img = BlobToImage((*img)->img_info, blob, size, e);
    CatchException(e);

    free(blob);
    rewind(fh);
    return 0;
}

void unload_image(image *img) {
    if (*img != NULL) {
        DestroyImage((*img)->img);
        DestroyImageInfo((*img)->img_info);
        free(*img);
        *img = NULL;
    }
}

int image_width(image img) {
    return img->img->columns;
}

int image_height(image img) {
    return img->img->rows;
}

bool get_pixel(image img, int x, int y, struct color *col) {
    PixelInfo pix;
    GetOneVirtualPixelInfo(img->img, 0, x, y, &pix, e);
    CatchException(e);
    col->r = pix.red / 65535 * 255;
    col->g = pix.green / 65535 * 255;
    col->b = pix.blue / 65535 * 255;
    col->a = pix.alpha / 65535 * 255;
    return true;
}
