/*----------------------------------------------------------------------
 * Ossuary:
 * A DOS port of the ZX Spectrum rogue-like.
 * Game handling header.
 * 
 * Copyright (C) Damian Gareth Walker 2013, 2020.
 * Created: 22-Jun-2020.
 */

#ifndef __GAME_H__
#define __GAME_H__

/*----------------------------------------------------------------------
 * Defined Constants.
 */

/* key code constants */
#define KEY_UP 0x8048
#define KEY_KEYPAD_UP 0x38
#define KEY_DOWN 0x8050
#define KEY_KEYPAD_DOWN 0x32
#define KEY_LEFT 0x804b
#define KEY_KEYPAD_LEFT 0x34
#define KEY_RIGHT 0x804d
#define KEY_KEYPAD_RIGHT 0x36
#define KEY_FIRE 0x0020

/*----------------------------------------------------------------------
 * Public Level Function Declarations.
 */

/**
 * Initialise some game variables.
 */
void init_game (void);

/**
 * Generate the hero.
 */
void generate_hero (void);

/**
 * Initialise the inventory.
 */
void init_inventory (void);

/**
 * Show the player stats.
 */
void show_stats (void);

/**
 * Show the gold counter.
 */
void show_gold (void);

/**
 * Show the inventory.
 */
void show_inventory (void);

/**
 * Ask the player for their name.
 */
void ask_name (void);

/**
 * Initialise the map for a level
 */
void generate_map (void);

/**
 * Place the hero on the current level
 */
void place_hero (void);

/**
 * Place the level's objective (staircase or necromancer)
 */
void place_objective (void);

/**
 * Place the level's eight enemies
 */
void place_enemies (void);

/**
 * Place the level's items
 */
void place_items (void);

/**
 * Generate the drawn map from a new level.
 */
void generate_drawn_map (void);

/**
 * Show the level number
 */
void show_level (void);

/**
 * Show the map window
 */
void show_map (void);

/**
 * Get and process player action.
 * @returns 1 if the player successfully performed an action.
 * May return 0 if, for example, the player tried to walk into a wall
 * or selected some menu option that does not constitute an action.
 */
int player_action (void);

/**
 * Generate and process enemy action
 */
void enemy_action (void);

/**
 * Generate and process the necromancer's action.
 */
void necro_action (void);

/**
 * See if the level is over.
 * @returns 1 if the level is over, 0 if not.
 */
int level_is_over (void);

/**
 * See if the game is over.
 * @returns 1 if the game is over, 0 if not.
 */
int game_is_over (void);

/**
 * Return a code to say who won the game, if anyone.
 * @returns +1 if the hero won, -1 if the hero lost, 0 otherwise.
 */
int who_won (void);

/**
 * See if user wants to quit.
 * @returns 1 if the player wants to quit, 0 if not.
 */
int player_quit (void);

/**
 * Return the player name.
 * @returns pointer to player name.
 */
char *get_name (void);

/**
 * Return the player's gold
 * @returns the gold
 */
int get_gold (void);

/**
 * Return the level number.
 * @returns the current level number.
 */
int get_level (void);

#endif
