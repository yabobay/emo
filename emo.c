#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "image.h"

#define EMOJI_DIR "SerenityOS-RGI-emoji/"

void output_color_to_terminal(struct color, struct color);
void dump_image_to_terminal(image);
bool string_starts_with(const char*, const char*);
void load_and_print_image(const char*);

int main(int argc, char **argv) {
    if (argc != 2) {
    usage:
        puts("Usage: emo [🙂|random]");
        return 0;
    }

    if (chdir(EMOJI_DIR) == -1) {
        puts("Sorry, the emoji dir needs to be placed in your working directory.");
        return 1;
    }
    FILE *fh = fopen("LIST_OF_EMOJI.txt", "r");
    if (fh == NULL) {
        printf("Error reading emoji list: %s\n", strerror(errno));
        return 1;
    }

    int selection = -1;
    if (!strcmp(argv[1], "random")) {
        char c;
        int newlines = 0;
        while ((c = fgetc(fh)) != EOF)
            if (c == '\n')
                newlines++;
        srand(time(NULL) + clock());
        selection = rand() % newlines;
        fseek(fh, 0, SEEK_SET);
    }

    char *line = NULL, *word, *filename;
    size_t line_size = 0;
    int i = 0;
    while (i++, getline(&line, &line_size, fh) != -1) {
        if (i == selection || string_starts_with(line, argv[1])) {
            word = strtok(line, " ");
            while (word) {
                filename = word;
                word = strtok(NULL, " ");
            }
            filename[strlen(filename)-2] = '\0';
            break;
        }
    }

    if (!filename)
        goto usage;

    load_and_print_image(filename);
    free(line);
    fclose(fh);
    return 0;
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

void load_and_print_image(const char *filename) {
    image_init();
    image img;
    load_image(&img, filename);
    dump_image_to_terminal(img);
    unload_image(&img);
    image_fini();
}
