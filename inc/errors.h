/*----------------------------------------------------------------------
 * Ossuary:
 * A DOS port of the ZX Spectrum rogue-like.
 * Error handling header.
 * 
 * Copyright (C) Damian Gareth Walker 2013, 2020.
 * Created: 22-Jun-2020.
 */

#ifndef __ERRORS_H__
#define __ERRORS_H__

/*----------------------------------------------------------------------
 * Public Level Function Prototypes.
 */

/**
 * Exit the program on an error condition, showing the error.
 * @param exit_code is the code that determines what the error was.
 */
void exit_game (int exit_code);

#endif
