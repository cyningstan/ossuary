/* temporary font conversion utility */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graph.h>
#include "conio.h"
#include "cgalib.h"

/**
 * Main Program
 */
void main (void)
{
    Screen *screen;
    Bitmap *bitmap;
    Font *font;
    FILE *fp;
    int ch, r, c;

    /* initialise the screen */
    _setvideomode (4);
    screen = scr_create (4);

    /* show the font */
    if (! _registerfonts ("ossuary.fon")) {
        printf ("Cannot load font.\n");
        exit (0);
    }
    _setfont ("n1");
    _moveto (0, 0);
    _outgtext (" !\"#$%&'()*+,-./");
    _moveto (0, 8);
    _outgtext ("0123456789:;<=>?");
    _moveto (0, 16);
    _outgtext ("@ABCDEFGHIJKLMNO");
    _moveto (0, 24);
    _outgtext ("PQRSTUVWXYZ[\\]^_");

    getch ();

    /* extract all the characters into a font */
    if (! (font = fnt_create (32, 95))) {
        printf ("Cannot create font.\n");
        exit (1);
    }
    if (! (bitmap = bit_create (8, 8))) {
        printf ("Cannot create bitmap.\n");
        exit (1);
    }
    if (! (fp = fopen ("ossuary.fnt", "wb"))) {
        printf ("Cannot create font file ossuary.fnt.\n");
        exit (1);
    }
    if (! fwrite ("CGA100F", 8, 1, fp)) {
        printf ("Cannot write header to font file.\n");
        exit (1);
    }
    for (ch = 32; ch <= 95; ++ch) {
        r = (ch - 32) / 16;
        c = (ch - 32) % 16;
        scr_get (screen, bitmap, 8 * c, 8 * r);
        fnt_put (font, bitmap, ch);
    }
    fnt_write (font, fp);
    fclose (fp);

    /* clean up at the end */
    bit_destroy (bitmap);
    fnt_destroy (font);
    scr_destroy (screen);
}
