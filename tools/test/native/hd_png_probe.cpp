#include "OMediaHDPng.h"
#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
int main(int argc, char **argv) {
    if (argc != 3) return 3;
    omhd::DecodedPng image;
    const char *err = omhd::decode_png(argv[1], image, (size_t)atoi(argv[2]) * 1024 * 1024);
    if (err) { fprintf(stderr, "%s\n", err); return 2; }
    printf("%u %u %lu\n", image.width, image.height,
        (unsigned long)crc32(0, &image.pixels[0], (uInt)image.pixels.size()));
    return 0;
}
