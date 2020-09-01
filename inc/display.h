/*----------------------------------------------------------------------
 * Ossuary:
 * A DOS port of the ZX Spectrum rogue-like.
 * Screen handling header.
 * 
 * Copyright (C) Damian Gareth Walker 2013, 2020.
 * Created: 22-Jun-2020.
 */

#ifndef __SCREEN_H__
#define __SCREEN_H__

/*----------------------------------------------------------------------
 * Type definitions
 */

/* Bitmaps */
typedef struct bitmap Bitmap;

/*----------------------------------------------------------------------
 * Public Level Routine Prototypes.
 */

/**
 * Initialise the screen.
 * @param mono is 1 for monochrome graphics, 0 for colour.
 * @param palette is 1 for ugly palette, 0 for custom palette.
 */
int init_screen (int mono, int palette);

/**
 * Load the game graphics.
 */
int load_graphics (void);

/**
 * Display the updates to the text window.
 */
void display_text (void);

/**
 * Display a number in the gold statistic box.
 * @param gold is the amount of gold.
 */
void display_gold (int gold);

/**
 * Display one of the three hero stats.
 * @param stat_no is the stat number, 0..2.
 * @param stat_val is the value of the stat.
 * @param stat_mod is the modifier.
 */
void display_stat (int stat_no, int stat_val, int stat_mod);

/**
 * Display the hero's health.
 * @param health_val is the current health value.
 * @param health_max is the maximum health value.
 */
void display_health (int health_val, int health_max);

/**
 * Display an inventory slot.
 * @param slot is the slot number.
 * @param item is the item number.
 */
void display_inventory (int slot, int item);

/**
 * Highlight an inventory slot with the cursor.
 * @param slot is the slot number.
 */
void highlight_inventory (int slot);

/**
 * Display a message in the bottom text area
 * @param message is the message to print
 */
void display_message (char *message);

/*
 * Blank out the previous message.
 */
void blank_message (void);

/**
 * Display some text in the top text area.
 * @param message is the message to print
 */
void display_heading (char *message);

/**
 * Alter a tile on the drawn map
 * @param x is the X location to alter.
 * @param y is the Y location to alter.
 * @param value is the map square value.
 */
void update_map_tile (int x, int y, int value);

/**
 * Build and display the visible portion of the game map.
 * @param map is the whole map.
 * @param hero_x is the hero's X location.
 * @param hero_y is the hero's Y locaation.
 */
void display_map (int hero_x, int hero_y);

/**
 * Display a blank square in the map area.
 */
void blank_map (void);

/**
 * Get a keypress, adding 0x8000 for extended codes, and converting
 * upper case letters to lower case.
 * @returns the keypress
 */
int get_keypress (void);

/**
 * Clean up the display.
 */
void clean_up_display (void);

/**
 * Display a game over message and wait for a key.
 * @param line_1 is the first line of the message.
 * @param line_2 is the second line of the message.
 * The lines are assumed to be exactly 8 characters long.
 */
void display_game_over (char *line_1, char *line_2);

/**
 * Print a debug message at the bottom of the screen.
 * @param message is the message.
 */
void debug_message (char *message);

/**
 * Display a row of the high score table.
 * @param row is the row number.
 * @param name is the name.
 * @param gold is the gold.
 */
void display_score (int row, char *name, int gold, int highlight);

/**
 * Display the game keys.
 */
void display_keys (void);

/**
 * Noise for a hit in combat.
 */
void hit_noise (void);

/**
 * Noise for a kill in combat.
 */
void kill_noise (void);

/**
 * Noise for some inventory action.
 */
void inventory_noise (void);

/**
 * Noise for the clink of gold.
 */
void gold_noise (void);

/**
 * Noise for descending the stairs.
 */
void stairs_noise (void);


#endif
