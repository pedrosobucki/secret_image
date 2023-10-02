#include <stdio.h>
#include "lib_ppm.h"

void bin(unsigned n);
void hide_size(Pixel *pixel, unsigned int height, unsigned int width);
void hide_pixel(unsigned char **bs, Pixel tohide);
void retrieve_size(Pixel *pixel, unsigned int *height, unsigned int *width);
Pixel retrieve_pixel(unsigned char **bs);



int main()
{
    // SETUP
    char *base_name = "bulb.ppm";
    char *secret_name = "bulbs.ppm";
    char *encoded_name = "encoded.ppm";
    char *decoded_name = "decoded.ppm";

    Img data1, data2;
    Img *base = &data1, *secret = &data2;


    // ENCODING
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

    hide_size(base->pix, secret->height, secret->width);

    char *bp = base->pix + 4;

    printf("BASE width: %d, height: %d\nSECRET width: %d, height: %d\n", base->width, base->height, secret->width, secret->height);
    for (unsigned int i = 0; i < secret_size; i++) {
        hide_pixel(&bp, secret->pix[i]);
    }

    write_ppm(encoded_name, base);

    free_ppm(base);
    free_ppm(secret);



    // DECODING
    Img data3, data4;
    Img *encoded = &data3, *decoded = &data4;

    r = read_ppm(encoded_name, encoded);

    if (r != 0) {
        printf("Failed to read '%s'.\n", encoded_name);
        return -1;
    }

    unsigned int height, width;
    retrieve_size(encoded->pix, &height, &width);
    printf("retrieve size: h %u X w %u\n", height, width);

    r = new_ppm(decoded, width, height);

    if (r != 0) {
        printf("Failed to create '%s'.\n", decoded_name);
        return -1;
    }

    bp = encoded->pix + 4;

    for (unsigned int i = 0; i < height*width; i++) {
        decoded->pix[i] = retrieve_pixel(&bp);
    }

    write_ppm(decoded_name, decoded);

    free_ppm(encoded);
    free_ppm(decoded);
}

Pixel retrieve_pixel(unsigned char **bs)
{
    Pixel px;
    unsigned char *p = &px;
    unsigned char color, sinfo;

    for (unsigned int i=0; i < 3; i++) {
        color = 0;

        for (unsigned int j=0; j < 4; j++) {
            color <<= 2;
            sinfo = **bs & 0x3;  // retrieves 2 least meaningful bits from color
            color |= sinfo;
            (*bs)++;
        }

        *p = color;
        p++;
    }

    return px;
}

void retrieve_size(Pixel *pixel, unsigned int *height, unsigned int *width)
{
    unsigned int h = 0, w = 0;

    unsigned char sinfo; 
    unsigned char *px = pixel;

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

void hide_pixel(unsigned char **bs, Pixel tohide)
{
    unsigned char color, mask;
    unsigned char *hp = &tohide;
    unsigned int size = 12;

    for (unsigned int i=0; i < size; i++) {
        color = *(*bs);
        color &= 0xFC;
        mask = (*hp >> 2*(size-i-1)) & 0x3;
        *(*bs) = color | mask;
        (*bs)++;
    }
}

void hide_size(Pixel *pixel, unsigned int height, unsigned int width)
{
    height = height << 12; // shifts height left 12 bits
    unsigned int size = height | width; // creates size var concatenating heiht and width bits

    unsigned char mask, rgb; 
    unsigned char *px = pixel; // creates pointer to pixel with 1 byte size

    for (unsigned int i=0; i < 12; i++) {   // since we store 2 secret bits in every byte, we iterate 12 times for 24 bits stored
        rgb = *(px+i);  // stores current pixel ptr value (points to specific color value)
        rgb &= 0xFC;    // clears last 2 bits of color value
        mask = (size >> (22 - 2*i)) & 0x3;  // creates mask with only 2 least significant bits from size
        *(px+i) = rgb | mask;   // stores OR between rgb and 2 size bits in color value
    }
}