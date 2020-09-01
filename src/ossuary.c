/*----------------------------------------------------------------------
 * Ossuary:
 * A DOS port of the ZX Spectrum rogue-like.
 * Main game source file.
 * 
 * Copyright (C) Damian Gareth Walker 2013, 2020.
 * Created: 22-Jun-2020.
 */

/* included compiler headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <graph.h>
#include <time.h>

/* included project headers */
#include "display.h"
#include "game.h"
#include "scores.h"
#include "errors.h"

/*----------------------------------------------------------------------
 * File-level variables.
 */

/** @var mono is 1 if monochrome graphics were requested. */
static int mono;

/** @var palette is 1 if standard palette was requested */
static int palette;

/*----------------------------------------------------------------------
 * Level 2 Routines
 */

/**
 * Check that the beta test version is not expired.
 * @param expiry is the expiry date, YYYYMMDD format.
 * @returns 1 if the beta is not expired.
 */
static int beta_time_check (char *expiry)
{
    time_t time_of_day;
    struct tm *full_time;
    char time_text[9];
    time_of_day = time (NULL);
    full_time = gmtime (&time_of_day);
    sprintf (time_text, "%04d%02d%02d", 1900 + full_time->tm_year,
        1 + full_time->tm_mon, full_time->tm_mday);
    return (strcmp (time_text, expiry) < 0);
}

/**
 * Initialise options on according to the command line.
 */
static int init_options (int argc, char **argv)
{
    /* local variables */
    int a; /* argument counter */
    
    /* check the arguments */
    for (a = 1; a < argc; ++a)
        if (! strcmp (argv[a], "-m"))
            mono = 1;
        else if (! strcmp (argv[a], "-p"))
            palette = 1;
        else
            return 0;

    /* all successful */
    return 1;
}

/*----------------------------------------------------------------------
 * Level 1 Routines
 */

/**
 * Initialise the program.
 * @returns 1 if successful, 0 if not.
 */
static int initialise (int argc, char **argv)
{
    if (! (beta_time_check ("99999999"))) exit_game (5);
    if (! (init_options (argc, argv))) exit_game (4);
    if (! (init_screen (mono, palette))) exit_game (1);
    if (! (load_graphics ())) exit_game (3);
    load_scores ();
    srand (time (NULL));
    return 1;
}

/**
 * Create a new game.
 */
static void new_game (void)
{
    init_game ();
    generate_hero ();
    init_inventory ();
    show_stats ();
    show_gold ();
    show_inventory ();
    show_scores ();
    ask_name ();
}

/**
 * Create a new level.
 */
static void new_level (void)
{
    generate_map ();
    place_hero ();
    place_objective ();
    place_enemies ();
    place_items ();
    generate_drawn_map ();
    show_level ();
    show_map ();
    blank_message ();
    if (get_level () == 1)
        display_message ("PRESS ? FOR KEYS");
}

/**
 * Play through a single level.
 * @returns 1 if the game is over, 0 if not.
 */
static int play_level (void)
{
    /* Continue moving player and enemies until the level is over */
    do {
        player_action ();
        if (! level_is_over ()) {
            enemy_action ();
            necro_action ();
        }
        show_map ();
    } while (! level_is_over ());

    /* reset the keyboard and return */
    return game_is_over ();
}

/**
 * Do the End Game things: high score table and play again query.
 * returns 1 if the player wants to finish, 0 if not.
 */
static int end_game (void)
{
    switch (who_won ()) {
        case -1:
            display_game_over ("YOU HAVE", "  DIED  ");
            break;
        case +1:
            display_game_over ("YOU HAVE", "  WON!  ");
            add_score (get_name (), get_gold ());
            break;
    }
    return player_quit ();
}

/**
 * Clean up when the program is finished.
 */
static void clean_up (void)
{
	save_scores ();
    clean_up_display ();
}

/*----------------------------------------------------------------------
 * Level 0 Routines
 */

/**
 * Main Program.
 * @param argc is the number of command line arguments.
 * @param argv is the command line arguments.
 */
void main (int argc, char **argv)
{
    /* local variables */
    int game_finished, /* 1 if user quits, wins or loses the game */
        program_finished; /* 1 if user quits the program */

    /* main program control */
    initialise (argc, argv);
    do {
        new_game ();
        do {
            new_level ();
            game_finished = play_level ();
        } while (! game_finished);
        program_finished = end_game ();
    } while (! program_finished);
    clean_up ();
}
