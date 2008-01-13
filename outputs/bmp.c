#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../vector.h"

typedef struct {
    uint8_t id[2];
    uint32_t filesize;
    uint32_t reserved;
    uint32_t h_size;
    uint32_t i_size;
    uint32_t width;
    uint32_t height;
    uint16_t bi_planes;
    uint16_t bits;
    uint32_t bi_compress;
    uint32_t bi_size_image;
    uint32_t bi_x_ppm;
    uint32_t bi_y_ppm;
    uint32_t bi_clr_used;
    uint32_t bi_clr_import;
} bmp_header_t;

int write_img( int w, int h, color ***image, char *file ) {
    bmp_header_t bh;
    FILE *bmpfile;
    int bpl, line, i, j;
    char *linebuf;

    bpl = w * 3;    /* for a 24bit image */
    if( bpl & 0x0003 ) {
        bpl |= 0x0003;
        ++bpl;
    }

    memset( (char*)&bh, 0, sizeof( bmp_header_t ) );
    memcpy( bh.id, "BM", 2 );

    bh.h_size = 54L;
    bh.i_size = 0x28L;
    bh.width = w;
    bh.height = h;
    bh.bi_planes = 1;
    bh.bits = 24;
    bh.bi_compress = 0;

    bh.filesize = bh.h_size + (long)bpl*bh.height;

    bmpfile = fopen( file, "wb" );
    if( bmpfile == NULL ) {
        fprintf( stderr, "*** error opening bitmap file for writing\n" );
        return -1;
    }

    /* write the header */
    fwrite( &bh.id, 1, 2, bmpfile );
    fwrite( &bh.filesize, 4, 1, bmpfile );
    fwrite( &bh.reserved, 2, 2, bmpfile );
    fwrite( &bh.h_size, 4, 1, bmpfile );
    fwrite( &bh.i_size, 4, 1, bmpfile );
    fwrite( &bh.width, 4, 1, bmpfile );
    fwrite( &bh.height, 4, 1, bmpfile );
    fwrite( &bh.bi_planes, 2, 1, bmpfile );
    fwrite( &bh.bits, 2, 1, bmpfile );
    fwrite( &bh.bi_compress, 4, 1, bmpfile );
    fwrite( &bh.bi_size_image, 4, 1, bmpfile );
    fwrite( &bh.bi_x_ppm, 4, 1, bmpfile );
    fwrite( &bh.bi_y_ppm, 4, 1, bmpfile );
    fwrite( &bh.bi_clr_used, 4, 1, bmpfile );
    fwrite( &bh.bi_clr_import, 4, 1, bmpfile );

    /* write the image */
    linebuf = (char*)calloc( 1, bpl );
    if( linebuf == NULL ) {
        fprintf( stderr, "*** error allocating memory for file io\n" );
        return -1;
    }

    for( line = h - 1; line >= 0; line-- ) {
        for( i = 0, j = 0; i < w && j < bpl; i++, j+=3 ) {
            /* bitmaps are stored BGR */
            linebuf[j] = (char)(image[line][i]->z*255.0f);
            linebuf[j+1] = (char)(image[line][i]->y*255.0f);
            linebuf[j+2] = (char)(image[line][i]->x*255.0f);
        }
        fwrite( linebuf, 1, bpl, bmpfile );
    }

    /* free the line buffer memory */
    free( linebuf );

    fclose( bmpfile );

    return 0;
}
