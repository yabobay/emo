#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <assert.h>

#include <unicode/utf8.h>

#include "image.h"

#define BORK(filename) {\
            printf("Couldn't open %s: %s\n", filename, strerror(errno));\
            exit(1);\
        }

#define TRY_TO_PRINT_EMOJI(filename, nope_zone) {\
    fh = fopen(filename, "rb");\
    if (!fh) {\
        if (errno == ENOENT)\
            /* TODO: if it is emoji, say that there's no image for that emoji */ \
            goto nope_zone;\
        BORK(filename);\
    }\
    load_image_from_file_handle(&img, fh);\
    fclose(fh);\
    dump_image_to_terminal(img);\
}

void output_color_to_terminal(struct color, struct color);
void dump_image_to_terminal(image);
bool string_starts_with(const char*, const char*);
char *guess_filename(const uint8_t *s);

int main(int argc, char **argv) {
    if (argc != 2) {
    usage:
        puts("Usage: emo [🙂|hex|random]");
        return 0;
    }

    int dir_size = 50;
    char *dir = malloc(dir_size * sizeof(char));
    int new_size = snprintf(dir, dir_size, "%s/.local/share/emo", getenv("HOME")) + 1;
    if (new_size > dir_size) {
        dir = realloc(dir, new_size * sizeof(char));
        snprintf(dir, new_size, "%s/.local/share/emo", getenv("HOME"));
    }

    if (chdir(dir) == -1) {
        // TODO: create the directory if errno == ENOENT
        printf("Error changing directory to %s: %s\n", dir, strerror(errno));
        return 1;
    }

    if (chdir("emoji") == -1) {
        printf("No 'emoji' directory found at %s.\n", dir);
        return 1;
    }

    image_init();
    image img = NULL;
    char *filename = NULL;
    FILE *fh = NULL;

    if (!strcmp(argv[1], "hex")) {
        puts("No you dummy! Like, type in the hex code of the emoji! 🤦‍♀️");
        goto cleanup;
    } else if (!strcmp(argv[1], "random")) {
        char *cwd = alloca(100);
        if (!getcwd(cwd, 100)) {
            puts(strerror(errno));
            return 1;
        }

        DIR *dir = opendir(cwd);
        if (!dir)
            BORK(cwd);

        struct dirent *d;
        int nfiles = 0;
        while (errno = 0, d = readdir(dir)) {
            if (d->d_type != DT_UNKNOWN && d->d_type != DT_REG)
                continue;
            ++nfiles;
        }
        if (errno) {
            puts(strerror(errno));
            return 1;
        }

        srand(time(NULL) + clock());
        int selection = rand() % nfiles;
        int original_selection = selection;

        rewinddir(dir);
        int i = 0;
        while (errno = 0, d = readdir(dir)) {
            if (d->d_type != DT_UNKNOWN && d->d_type != DT_REG)
                continue;
            if (i++ == selection)
                break;
        }
        if (errno) {
            puts(strerror(errno));
            return 1;
        }

        IMAGE_ERROR ie = load_image_from_filename(&img, d->d_name);
        switch (ie) {
        case COOL:
            dump_image_to_terminal(img);
            break;
        case THAT_WASNT_AN_IMAGE:
            printf("File \"%s\" in the emoji directory isn't an image!\n", d->d_name);
            break;
        default:
            printf("Uh-oh, error #%d :(\n", ie);
        }

        closedir(dir);
        goto cleanup;
    }

    // Maybe it's an emoji:

    filename = guess_filename(argv[1]);
    TRY_TO_PRINT_EMOJI(filename, not_emoji);
    goto cleanup;

not_emoji:
    // Maybe it's a hex code:

    UChar32 hex;
    hex = strtol(argv[1], NULL, 16);
    if (!hex)
        goto not_hex;
    filename = malloc(25); // ¯\_(ツ)_/¯
    sprintf(filename, "U+%04X.png", hex);
    TRY_TO_PRINT_EMOJI(filename, not_hex);
    goto cleanup;

not_hex:
    goto usage;

cleanup:
    unload_image(&img);
    image_fini();
    if (filename)
        free(filename);
    return 0;
}

// requires heap string
void strappend(char **str, size_t *size, const char *format, ...) {
    va_list args, backup_args;
    va_start(args, format);
    va_copy(backup_args, args);
    int len = strlen(*str);
    int written = vsnprintf(*str + len, *size + 1 - len, format, args);
    if (written > *size - len) {
        *size += written + 1;
        *str = realloc(*str, sizeof(char) * *size);
        vsnprintf(*str + len, *size + 1 - len, format, backup_args);
    }
    va_end(args);
    va_end(backup_args);
}

char *guess_filename(const uint8_t *s) {
    char *filename = strdup("");
    size_t flen = 0;
    int32_t i = 0, length = strlen(s);
    UChar32 c;
    while (true) {
        U8_GET(s, 0, i, length, c);
        if (!c) break;
        if (i > 0)
            strappend(&filename, &flen, "_");
        strappend(&filename, &flen, "U+%04X", c);
        U8_FWD_1(s, i, length);
    }
    strappend(&filename, &flen, ".png");
    return filename;
}

void output_color_to_terminal(struct color top, struct color bottom) {
    if (top.a) {
        printf("\x1b[38;2;%u;%u;%um", top.r, top.g, top.b);
        if (bottom.a)
            printf("\x1b[48;2;%u;%u;%um", bottom.r, bottom.g, bottom.b);
        printf("▀\x1b[0m");
    } else if (bottom.a)
        printf("\x1b[38;2;%u;%u;%um▄\x1b[0m", bottom.r, bottom.g, bottom.b);
    else
        putchar(' ');
}

void dump_image_to_terminal(image img) {
    assert(img != NULL);
    for (int y = 0; y < image_height(img)-1; y += 2) {
        for (int x = 0; x < image_width(img); x++) {
            struct color col1, col2;
            get_pixel(img, x, y, &col1);
            get_pixel(img, x, y+1, &col2);
            output_color_to_terminal(col1, col2);
        }
        putchar('\n');
    }
}

bool string_starts_with(const char *a, const char *b) {
    size_t len = strlen(b);
    if (len > strlen(a))
        return false;
    for (size_t i = 0; i < len; i++)
        if (a[i] != b[i])
            return false;
    return true;
}
