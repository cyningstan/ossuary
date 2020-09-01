/*----------------------------------------------------------------------
 * Ossuary:
 * A DOS port of the ZX Spectrum rogue-like.
 * Score handling header.
 * 
 * Copyright (C) Damian Gareth Walker 2013, 2020.
 * Created: 16-Jul-2020.
 */

#ifndef __SCORES_H__
#define __SCORES_H__

/*----------------------------------------------------------------------
 * Public Level Function Declarations.
 */

/**
 * Load or generate the high score table.
 */
void load_scores (void);

/**
 * Show the high score table.
 */
void show_scores (void);

/**
 * Save the scores at the end of the game (if changed)
 */
void save_scores (void);

/**
 * Consider adding a score to the high score table.
 * @param name is the player name.
 * @param gold is the amount of gold.
 */
void add_score (char *name, int gold);

#endif
