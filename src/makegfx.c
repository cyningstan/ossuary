/*----------------------------------------------------------------------
 * Ossuary:
 * A DOS port of the ZX Spectrum rogue-like.
 * Graphics file generation.
 * 
 * Copyright (C) Damian Gareth Walker 2013, 2020.
 * Created: 22-Jun-2020.
 */

/* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <memory.h>
#include "cgalib.h"

/*----------------------------------------------------------------------
 * File Level Variables.
 */

/** @var logo is the Cyningstan logo bitmap */
static Bitmap *logo;

/** @var bitmap is the collection of bitmaps for the graphics file. */
static Bitmap *bitmap [30];

/** @var font is the font */
static Font *font;

/** @var infile is the input file name */
static char *infile;

/** @var outfile is the output file name */
static char *outfile;

/** @var pic is the pixel data from the pic file */
static char *pic;

/*----------------------------------------------------------------------
 * Level 1 Routines.
 */

/**
 * Look for an input and an output file on the command line.
 * @param argc is the number of command line arguments.
 * @param argv is the command line arguments.
 */
static void parse_args (int argc, char **argv)
{
    /* check for the correct number of arguments */
    if (argc != 3) {
        printf ("Usage: %s INFILE OUTFILE\n", argv [0]);
        exit (0);
    }

    /* extract the filenames */
    infile = argv [1];
    outfile = argv [2];
}

/**
 * Load the input file, which will be in PIC format.
 */
static void load_image (void)
{
    FILE *input;
    char buffer[7];
    if (! (input = fopen (infile, "rb"))) exit (1);
    if (! (fread (buffer, 7, 1, input))) exit (2);
    if (! (pic = malloc (16192))) exit (3);
    if (! (fread (pic, 16192, 1, input))) exit (4);
    fclose (input);
}

/**
 * Extract the graphics from the input file.
 */
static void extract_graphics (void)
{
    int row, col, x, y, bit_no;
    Screen *screen;
    Bitmap *charbit;

    /* show screen */
    screen = scr_create (5);
    scr_palette (screen, 2, 14);
    _fmemcpy ((void far *) 0xb8000000, pic, 16192);

    /* extract the tiles */
    bit_no = 1;
    for (row = 0; row < 5; ++row)
        for (col = (row == 3 || row == 4) ? 8 : 0; col < 9; ++col) {
            x = 88 + 16 * col;
            y = 12 + 16 * row;
            if (! (bitmap[bit_no] = bit_create (16, 16))) {
                scr_destroy (screen);
                printf ("Cannot reserve memory for bitmap.\n");
                exit (1);
            }
            scr_get (screen, bitmap[bit_no], x, y);
            ++bit_no;
        }
    
    /* extract the font */
    if (! (font = fnt_create (32, 95))) {
        scr_destroy (screen);
        printf ("Out of memory creating font.\n");
        exit (1);
    }
    if (! (charbit = bit_create (8, 8))) {
        scr_destroy (screen);
        printf ("Out of memory creating bitmap for character.\n");
        exit (1);
    }
    for (row = 0; row < 4; ++row)
        for (col = 0; col < 16; ++col) {
            scr_get (screen, charbit, 88 + 8 * col, 60 + 8 * row);
            fnt_put (font, charbit, 32 + col + 16 * row);
        }
    bit_destroy (charbit);
    
    /* extract the logo */
    if (! (logo = bit_create (128, 16))) {
        scr_destroy (screen);
        printf ("Cannot reserve memory for logo.\n");
        exit (1);
    }
    scr_get (screen, logo, 88, 92);
    
    /* extract the background screen */
    scr_ink (screen, 2);
    scr_box (screen, 88, 12, 144, 144);
    if (! (bitmap[0] = bit_create (320, 200))) {
        scr_destroy (screen);
        exit (6);
    }
    scr_get (screen, bitmap[0], 0, 0);

    /* return to text mode */
    scr_destroy (screen);
}

/**
 * Save the graphics to the output file
 */
static void save_graphics (void)
{
    FILE *output;
    int c;
    if (! (output = fopen (outfile, "wb"))) exit (7);
    fwrite ("OSS100D", 8, 1, output);
    bit_write (logo, output);
    for (c = 0; c < 30; ++c)
        bit_write (bitmap[c], output);
    fnt_write (font, output);
    fclose (output);
}

/*----------------------------------------------------------------------
 * Level 0 Routines.
 */

/**
 * Main Program.
 * @param argc is the number of command line arguments.
 * @param argv is the command line arguments.
 */
void main (int argc, char **argv)
{
    parse_args (argc, argv);
    load_image ();
    extract_graphics ();
    save_graphics ();
}
