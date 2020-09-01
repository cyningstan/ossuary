/*----------------------------------------------------------------------
 * Ossuary:
 * A DOS port of the ZX Spectrum rogue-like.
 * Score handling module.
 * 
 * Copyright (C) Damian Gareth Walker 2013, 2020.
 * Created: 16-Jul-2020.
 */

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "display.h"
#include "scores.h"

/*----------------------------------------------------------------------
 * Data Definitions.
 */

/** @struct score is a single high score. */
typedef struct score Score;
struct score {
    char name[11]; /* player name */
    int gold; /* gold collected */
};

/*----------------------------------------------------------------------
 * File Level Variables.
 */

/** @var scores is the high score table. */
static Score scores[18];

/** @var last_score is the row of the last score inserted */
static int last_score = -1;

/** @var scores_changed is 1 if the scores have changed */
static int scores_changed = 0;

/*----------------------------------------------------------------------
 * Level 1 Private Functions.
 */

/**
 * Load scores from the high score file.
 * Returns 0 on success.
 */
static int read_scores (void)
{
    /* local variables */
    FILE *input; /* input file handle */
    char name[11], /* name read from input file */
        header[8], /* header read from input file */
        len; /* name length read from input file */
    int gold, /* gold read from input file */
        c; /* score counter */

	/* attempt to open file */
	if (! (input = fopen ("ossuary.sco", "rb")))
	    return 0;

    /* attempt to read header */
    if (! fread (header, 8, 1, input)) {
        fclose (input);
        return 0;
    }
    
    /* verify header */
    if (strcmp (header, "OSS100S")) {
        fclose (input);
        return 0;
    }
    
    /* read the scores */
    for (c = 0; c < 18; ++c) {
        if (! fread (&len, 1, 1, input)) {
            fclose (input);
            return 0;
        }
        if (! fread (name, len, 1, input)) {
            fclose (input);
            return 0;
        }
        name[(int) len] = '\0';
        if (! fread (&gold, 2, 1, input)) {
            fclose (input);
            return 0;
        }
        strcpy (scores[c].name, name);
        scores[c].gold = gold;
    }
    
    /* successful! */
    fclose (input);
    return 1;
}

/**
 * Generate new high score table.
 */
static void create_scores (void)
{
    /* local variables */
    int c; /* score counter */

    /* fill score table with Cyningstan */
    for (c = 0; c < 18; ++c) {
        strcpy (scores[c].name, "CYNINGSTAN");
        scores[c].gold = 0;
    }
    
    /* these scores want saving */
    scores_changed = 1;
}

/*----------------------------------------------------------------------
 * Public Level Functions.
 */

/**
 * Load or generate the high score table.
 */
void load_scores (void)
{
    if (! read_scores ())
        create_scores ();
}

/**
 * Show the high score table.
 */
void show_scores (void)
{
    /* local variables */
    int c; /* score counter */
    
    /* show the display */
    display_heading (" SCORES ");
    for (c = 0; c < 18; ++c)
        display_score (c, scores[c].name, scores[c].gold,
            c == last_score);
    display_text ();
}

/**
 * Save the scores at the end of the game (if changed)
 */
void save_scores (void)
{
	/* local variables */
    FILE *output; /* output file handle */
    char len; /* name length to write to file */
    int c; /* score counter */

    /* if the scores haven't changed, don't save them */
    if (! scores_changed)
        return;
    
    /* create the file and write the header */
    if (! (output = fopen ("OSSUARY.SCO", "wb")))
        return;
    fwrite ("OSS100S", 8, 1, output);
    
    /* loop through the scores */
    for (c = 0; c < 18; ++c) {
        len = strlen (scores[c].name);
        fwrite (&len, 1, 1, output);
        fwrite (scores[c].name, len, 1, output);
        fwrite (&scores[c].gold, 2, 1, output);
    }

    /* close the file */
    fclose (output);
}

/**
 * Consider adding a score to the high score table.
 * @param name is the player name.
 * @param gold is the amount of gold.
 */
void add_score (char *name, int gold)
{
    /* local variables */
    int c; /* score counter */
    
    /* don't insert a score if they have no name */
    if (! *name)
        return;

    /* look for a slot to insert the score */
    for (c = 0; c < 18; ++c)
        if (gold > scores[c].gold)
            break;
    
    /* return now if score was not high enough */
    if (c == 18)
        return;

    /* shuffle lower scores down the table */
    last_score = c;
    for (c = 17; c > last_score; --c)
        scores[c] = scores[c - 1];
    
    /* insert this score */
    strcpy (scores[last_score].name, name);
    scores[last_score].gold = gold;
    scores_changed = 1;
}
