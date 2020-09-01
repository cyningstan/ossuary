/*----------------------------------------------------------------------
 * Ossuary:
 * A DOS port of the ZX Spectrum rogue-like.
 * Screen handling module.
 * 
 * Copyright (C) Damian Gareth Walker 2013, 2020.
 * Created: 22-Jun-2020.
 */

/* compiler includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <i86.h>
#include <dos.h>
#include <sys/timeb.h>

/* project includes */
#include "cgalib.h"
#include "display.h"

/*----------------------------------------------------------------------
 * File level variables.
 */

/** @var screen is the CGALIB screen structure */
static Screen *screen;

/** @var mapimg is the bitmap that holds the drawn map */
static Bitmap *mapimg = NULL;

/** @var mapwin is the bitmap that holds the visible map section */
static Bitmap *mapwin = NULL;

/** @var txtwin is the bitmap that holds text screens */
static Bitmap *txtwin = NULL;

/** @var logo is the Cyningstan logo briefly displayed on the screen */
static Bitmap *logo;

/** @var bitmaps is the collection of bitmaps used in the game. */
static Bitmap *bitmaps[30];

/** @var font is the font used in the game */
static Font *font;

/** @var item_bitmaps links item numbers to the appropriate bitmap */
static int item_bitmaps[16] = {
    22, 12, 13, 10, 11, 14, 15, 16, 17, 18, 21, 20, 19, 22, 22, 23
};

/** @var last_message is the last message printed. */
static char last_message[17];

/*----------------------------------------------------------------------
 * Public Level Routines.
 */

/**
 * Initialise the screen.
 */
int init_screen (int mono, int palette)
{
    /* initialise the screen using CGALIB */
    screen = scr_create (mono ? 6 : 4);
    if (! palette)
        scr_palette (screen, 2, 14);
    scr_ink (screen, 2);
    scr_box (screen, 0, 0, 320, 200);

    /* return success */
    return 1;
}

/**
 * Clean up the display.
 */
void clean_up_display (void)
{
    scr_destroy (screen);
}

/**
 * Load the game graphics.
 */
int load_graphics (void)
{
    /* local variables */
    FILE *input; /* input file handle */
    int count; /* graphics counter */
    char header[8]; /* header buffer */

    /* open the file and read and check the header */
    if (! (input = fopen ("ossuary.gfx", "rb"))) return 0;
    if (! (fread (header, 8, 1, input))) return 0;
    if (header[7] != '\0') return 0;
    if (strcmp (header, "OSS100D")) return 0;
    
    /* load and display the logo */
    if (! (logo = bit_read (input)))
        return 0;
    scr_put (screen, logo, 96, 92, DRAW_PSET);
    bit_destroy (logo);

    /* load each of the graphic images */
    for (count = 0; count < 30; ++count)
        if (! (bitmaps[count] = bit_read (input)))
            return 0;

    /* load the font */
    if (! (font = fnt_read (input)))
        return 0;
    scr_font (screen, font);

    /* create the map bitmaps */
    if (! (mapimg = bit_create (256, 256)))
        return 0;
    if (! (mapwin = bit_create (144, 144)))
        return 0;
    if (! (txtwin = bit_create (144, 144)))
        return 0;
    
    /* initialise the text window */
    bit_ink (txtwin, 2);
    bit_box (txtwin, 0, 0, 144, 144);
    bit_font (txtwin, font);

    /* put the image on the screen and return success */
    sleep (3);
    scr_put (screen, bitmaps[0], 0, 0, DRAW_PSET);
    return 1;
}

/**
 * Display the updates to the text window.
 */
void display_text (void)
{
    scr_put (screen, txtwin, 88, 12, DRAW_PSET);
}

/**
 * Display a number in the gold statistic box.
 * @param gold is the amount of gold.
 */
void display_gold (int gold)
{
    char gold_string[5];
    sprintf (gold_string, "%04d", gold);
    scr_paper (screen, 2);
    scr_ink (screen, 3);
    scr_print (screen, 40, 84, gold_string);
}

/**
 * Display one of the three hero stats.
 * @param stat_no is the stat number, 0..2.
 * @param stat_val is the value of the stat.
 * @param stat_mod is the modifier.
 */
void display_stat (int stat_no, int stat_val, int stat_mod)
{
    /* local variables */
    char val_string[3];
    char mod_string[3];
    
    /* build the strings */
    sprintf (val_string, "%02d", stat_val);
    sprintf (mod_string, "+%c", '0' + stat_mod);

    /* print the value */
    scr_paper (screen, 2);
    scr_ink (screen, 3);
    scr_print (screen, 56, 12 + 16 * stat_no, val_string);
    
    /* print the modifier */
    scr_ink (screen, 1);
    scr_print (screen, 56, 20 + 16 * stat_no, mod_string);
}

/**
 * Display the hero's health.
 * @param health_val is the current health value.
 * @param health_max is the maximum health value.
 */
void display_health (int health_val, int health_max)
{
    /* local variables */
    char val_string[3];
    char max_string[3];
    unsigned long int far *overline;
    
    /* build the strings */
    sprintf (val_string, "%02d", health_val);
    sprintf (max_string, "%02d", health_max);

    /* print the value */
    scr_paper (screen, 2);
    scr_ink (screen, health_val < 10 ? 0 : 3);
    scr_print (screen, 56, 60, val_string);
    
    /* print the max value */
    scr_ink (screen, 1);
    scr_print (screen, 56, 68, max_string);

    /* superimpose a line over the max value */
    overline = (unsigned long int far *) 0xb8000aae;
    *overline = 0x56555595;

}

/**
 * Display an inventory slot.
 * @param slot is the slot number.
 * @param item is the item number.
 */
void display_inventory (int slot, int item)
{
    /* local variables */
    int x, y;
    int b;
    
    /* ascertain screen position and bitmap number */
    x = 248 + 16 * (slot % 2);
    y = 12 + 16 * (int) (slot / 2) + 8 * (slot > 1);
    b = item_bitmaps[item];

    /* put the bitmap on the screen */
    scr_put (screen, bitmaps[b], x, y, DRAW_PSET);
}

/**
 * Highlight an inventory slot with the cursor.
 * @param slot is the slot number.
 */
void highlight_inventory (int slot)
{
    /* local variables */
    int x, y;
    
    /* ascertain screen position */
    x = 248 + 16 * (slot % 2);
    y = 12 + 16 * (int) (slot / 2) + 8 * (slot > 1);

    /* show the cursor in the right position */
    scr_put (screen, bitmaps[28], x, y, DRAW_AND);
}

/**
 * Display a prompt in the bottom text area
 * @param message is the message to print
 */
void display_message (char *message)
{
    /* if a message was just printed, delay */
    if (*last_message) {
        scr_paper (screen, 1);
        scr_ink (screen, 2);
        scr_print (screen, 96, 156, last_message);
        sleep (1);
    }

    /* print the text */
    scr_paper (screen, 1);
    scr_ink (screen, 0);
    scr_print (screen, 96, 156, message);
    
    /* remember the message */
    strcpy (last_message, message);
}

/*
 * Blank out the previous message.
 */
void blank_message (void)
{
    scr_ink (screen, 1);
    scr_box (screen, 96, 156, 128, 8);
    *last_message = 0;
}

/**
 * Display some text in the top text area.
 * @param message is the message to print
 */
void display_heading (char *message)
{
    /* print the text */
    scr_paper (screen, 1);
    scr_ink (screen, 0);
    scr_print (screen, 128, 4, message);
}

/**
 * Alter a tile on the drawn map
 * @param x is the X location to alter.
 * @param y is the Y location to alter.
 * @param value is the map square value.
 */
void update_map_tile (int x, int y, int value)
{
    /* local variables */
    int bm_index; /* which bitmap to show */

    /* work out the tile to display */
    if ((value & 0xf0) == 0x20) /* hero */
        bm_index = 1;
    else if (value < 4) /* wall */
        bm_index = 24 + value;
    else if ((value & 0xf0) == 0x10) /* floor, item, stairs */
        bm_index = item_bitmaps[value & 0xf];
    else if (value & 0x80) /* enemy */
        bm_index = (value & 0x70) / 0x10 + 2;
    else if (value == 0x30) /* necromancer */
    	bm_index = 29;
    else /* whoops! */
        bm_index = 22;

    /* copy the bitmap */
    bit_put (mapimg, bitmaps[bm_index], 16 * x, 16 * y, DRAW_PSET);
}

/**
 * Build and display the visible portion of the game map.
 * @param map is the whole map.
 * @param hero_x is the hero's X location.
 * @param hero_y is the hero's Y locaation.
 */
void display_map (int hero_x, int hero_y)
{
    /* local variables */
    int top, /* map y coordinate at the top of the window */
        left; /* map x coordinate at the left of the window */

    /* work out the top left of the map */
    if (hero_x < 4) left = 0;
    else if (hero_x < 11) left = hero_x - 4;
    else left = 7;
    if (hero_y < 4) top = 0;
    else if (hero_y < 11) top = hero_y - 4;
    else top = 7;

    /* put the map on the screen */
    bit_get (mapimg, mapwin, 16 * left, 16 * top);
    scr_put (screen, mapwin, 88, 12, DRAW_PSET);
}

/**
 * Display a blank square in the map area.
 */
void blank_map (void)
{
    scr_ink (screen, 2);
    scr_box (screen, 88, 12, 144, 144);
}

/**
 * Get a keypress, adding 0x8000 for extended codes, and converting
 * upper case letters to lower case.
 * @returns the keypress
 * Keys are as follows:
 *    SPC Enter the menu.
 *    Esc Leave the game.
 *    I   Inventory.
 *    Q   Quaff a potion.
 *    E   Eat an apple.
 *    P   Pick up item.
 */
int get_keypress (void)
{
    /* local variables */
    int key;

    /* clear the keyboard buffer */
    while (kbhit ()) getch ();

    /* get the key */
    key = getch ();
    if (key == 0)
        key = getch () + 0x8000;
    if (key >= 'A' && key <= 'Z')
        key += 32;

    /* once a key is pressed, we don't need to delay last message */
    *last_message = 0;

    /* return the key */
    return key;
}

/**
 * Display a game over message and wait for a key.
 * @param line_1 is the first line of the message.
 * @param line_2 is the second line of the message.
 * The lines are assumed to be exactly 8 characters long.
 */
void display_game_over (char *line_1, char *line_2)
{
    display_message (" ESC TO RESTART ");
    scr_ink (screen, 0);
    scr_paper (screen, 2);
    scr_print (screen, 128, 76, line_1);
    scr_print (screen, 128, 84, line_2);
    while (getch () != 27);
    blank_message ();
}

/**
 * Print a debug message at the bottom of the screen.
 * @param message is the message.
 */
void debug_message (char *message)
{
    char buffer[41],
        *ch;
    sprintf (buffer, "%-40s", message);
    for (ch = buffer; *ch; ++ch)
        *ch = toupper (*ch);
    scr_ink (screen, 0);
    scr_paper (screen, 2);
    scr_print (screen, 0, 192, buffer);
    if (! getch ()) getch ();
    scr_ink (screen, 2);
    scr_box (screen, 0, 192, 320, 8);
}

/**
 * Display a row of the high score table.
 * @param row is the row number.
 * @param name is the name.
 * @param gold is the gold.
 */
void display_score (int row, char *name, int gold, int highlight)
{
    /* local variables */
    char text[19]; /* the text of this row */

    /* prepare the text and print it */
    sprintf (text, " %-10s  %04d ", name, gold);
    bit_paper (txtwin, 2);
    bit_ink (txtwin, highlight ? 0 : 3);
    bit_print (txtwin, 0, 8 * row, text);
}

/**
 * Display the game keys.
 */
void display_keys (void)
{
    /* display heading */
    display_heading ("  KEYS  ");
    display_message ("ESC BACK TO GAME");

    /* display subheadings */
    bit_paper (txtwin, 2);
    bit_ink (txtwin, 3);
    bit_print (txtwin, 0, 0, "WHILE MOVING HERO ");
    bit_print (txtwin, 0, 88, "WHILE IN INVENTORY");

    /* display the keys */
    bit_ink (txtwin, 0);
    bit_print (txtwin, 0, 8, ". ");
    bit_print (txtwin, 0, 16, "I ");
    bit_print (txtwin, 0, 24, "T ");
    bit_print (txtwin, 0, 32, "S ");
    bit_print (txtwin, 0, 40, "E ");
    bit_print (txtwin, 0, 48, "Q ");
    bit_print (txtwin, 0, 56, "N ");
    bit_print (txtwin, 0, 64, "X ");
    bit_print (txtwin, 0, 96, "W ");
    bit_print (txtwin, 0, 104, "P ");
    bit_print (txtwin, 0, 112, "C ");
    bit_print (txtwin, 0, 120, "D ");

    /* display descriptions */
    bit_ink (txtwin, 1);
    bit_print (txtwin, 16, 8, "WAIT A MOMENT...");
    bit_print (txtwin, 16, 16, "VIEW INVENTORY  ");
    bit_print (txtwin, 16, 24, "PICK UP ITEM    ");
    bit_print (txtwin, 16, 32, "DESCEND STAIRS  ");
    bit_print (txtwin, 16, 40, "EAT A NICE APPLE");
    bit_print (txtwin, 16, 48, "QUAFF A POTION  ");
    bit_print (txtwin, 16, 56, "START A NEW GAME");
    bit_print (txtwin, 16, 64, "EXIT TO DOS     ");
    bit_print (txtwin, 0, 72, "  ARROWS MOVE HERO");
    bit_print (txtwin, 0, 80, "  SPACE FOR MENU  ");
    bit_print (txtwin, 16, 96, "WIELD AN ITEM   ");
    bit_print (txtwin, 16, 104, "PUT AN ITEM AWAY");
    bit_print (txtwin, 16, 112, "CONSUME AN ITEM ");
    bit_print (txtwin, 16, 120, "DISCARD AN ITEM ");
    bit_print (txtwin, 0, 128, "  ARROWS SELECT   ");
    bit_print (txtwin, 0, 136, "  SPACE FOR MENU  ");

    /* wait for the ESC key */
    display_text ();
    while (getch () != 27);
    blank_message ();
}

/**
 * Noise for a hit in combat.
 */
void hit_noise (void)
{
    /* local variables */
    struct timeb start, /* start time for noise */
        now; /* current time */

    /* make the noise */
    ftime (&start);
    do {
        sound (128 + rand () % 64);
        ftime (&now);
    } while (now.millitm == start.millitm);
    nosound ();
}

/**
 * Noise for a kill in combat.
 */
void kill_noise (void)
{
    /* local variables */
    struct timeb start, /* start time for noise */
        now; /* current time */
    int pitch;

    /* make the noise */
    pitch = 128 + rand () % 64;
    ftime (&start);
    do {
        sound (pitch);
        pitch -= (rand () % pitch) / 32;
        ftime (&now);
    } while (now.millitm == start.millitm);
    nosound ();
}

/**
 * Noise for some inventory action.
 */
void inventory_noise (void)
{
    sound (256 + rand () % 128);
    delay (1);
    nosound ();
}

/**
 * Noise for the clink of gold.
 */
void gold_noise (void)
{
    /* local variables */
    struct timeb start, /* start time for noise */
        now; /* current time */

    /* make the noise */
    ftime (&start);
    do {
        sound (768 + rand () % 256);
        ftime (&now);
    } while (now.millitm == start.millitm);
    nosound ();
}

/**
 * Noise for descending the stairs.
 */
void stairs_noise (void)
{
    /* local variables */
    int pitch, /* descending pitch value */
        count; /* count the number of steps */

    /* make the noise */
    pitch = 128 + rand () % 64;
    for (count = 0; count < 4; ++count) {
        sound (pitch);
        delay (1);
        nosound ();
        delay (250);
        pitch -= (rand () % pitch) / 4;
    }
}
