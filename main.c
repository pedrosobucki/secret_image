#include <stdio.h>
#include "lib_ppm.h"

void hide_size(Pixel *pixel, unsigned int height, unsigned int width);
void hide_pixel(char *bs, Pixel tohide);

void retrieve_size(Pixel *pixel, unsigned int *height, unsigned int *width)
{
    unsigned int h = 0, w = 0;

    char mask, sinfo; 
    char *px = pixel;

    for (unsigned int i=0; i < 6; i++) {
        h <<= 2;
        sinfo = *px & 0x3;  // retrieves 2 least meaningful bits from color
        h |= sinfo;
        px++;
    }

    for (unsigned int i=0; i < 6; i++) {
        w <<= 2;
        sinfo = *px & 0x3;  // retrieves 2 least meaningful bits from color
        w |= sinfo;
        px++;
    }

    *height = h;
    *width = w;
}

void bin(unsigned n);

int main()
{
    char *base_name = "earth.ppm";
    char *secret_name = "british_summer.ppm";

    Img data1, data2;
    Img *base = &data1, *secret = &data2;

    int r = read_ppm(base_name, base) + read_ppm(secret_name, secret);

    if (r != 0) {
        printf("Failed to read '%s'.\n", base_name);
        return -1;
    }

    int base_size = base->height * base->width;
    int secret_size = secret->height * secret->width;

    if (
        base_size < ((secret_size+1 * 4))
        || secret->height > 4095
        || secret->width > 4095
    ) {
        printf("Secret size is too large.\n");
        return -1;
    }

    // for (unsigned int i=0; i < 4; i++) {
    //     printf("%02x %02x %02x\n", base->pix[i].r, base->pix[i].g, base->pix[i].b);
    // }

    hide_size(base->pix, secret->height, secret->width);

    unsigned char *bp = base->pix + 12;
    unsigned char s;

    printf("BASE width: %d, height: %d\n", base->width, base->height);
    printf("SECRET width: %d, height: %d\n", secret->width, secret->height);
    for (unsigned int i = 0; i < secret_size; i++)
    {
        hide_pixel(bp, secret->pix[i]);
    }

    char *encoded_name = "encoded.ppm";

    write_ppm(encoded_name, base);

    free_ppm(base);
    free_ppm(secret);

    r = read_ppm(encoded_name, base);

    if (r != 0) {
        printf("Failed to read '%s'.\n", encoded_name);
        return -1;
    }

    unsigned int height, width;
    retrieve_size(base->pix, &height, &width);

    printf("retrieve size: h %u X w %u\n", height, width);
}

void hide_pixel(char *bs, Pixel tohide)
{
    unsigned char color, mask;
    unsigned char *hp = &tohide;

    for (unsigned int i=0; i < sizeof(tohide)*4; i++) {
        color = *bs;
        color &= 0xFC;
        mask = *hp & 0x3;
        *bs = color | mask;
        *hp >>= 2;
        bs++;
    }
}

void hide_size(Pixel *pixel, unsigned int height, unsigned int width)
{
    height = height << 12; // shifts height left 12 bits
    unsigned int size = height | width; // creates size var concatenating heiht and width bits

    char mask, rgb; 
    char *px = pixel; // creates pointer to pixel with 1 byte size

    // printf("loop start ---------------\n");
    for (unsigned int i=0; i < 12; i++) {   // since we store 2 secret bits in every byte, we iterate 12 times for 24 bits stored
        rgb = *(px+i);  // stores current pixel ptr value (points to specific color value)
        rgb &= 0xFC;    // clears last 2 bits of color value
        mask = (size >> 22 - 2*i) & 0x3;  // creates mask with only 2 least significant bits from size
        // printf("%p(%u) -> %02x + %02x = %02x\n", px+i, i, *(px+i), mask, rgb | mask);
        *(px+i) = rgb | mask;   // stores OR between rgb and 2 size bits in color value
        // size >>= 2; // shifts size value 2 bits to the right, now storing the next 2 bits to be hiden in the least significant bits
    }
    // printf("loop end---------------\n");
}

void bin(unsigned n)
{
    unsigned i;
    for (i = 1 << 31; i > 0; i = i / 2)
        (n & i) ? printf("1") : printf("0");

    printf("\n");
}