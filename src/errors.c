/*----------------------------------------------------------------------
 * Ossuary:
 * A DOS port of the ZX Spectrum rogue-like.
 * Error handling module.
 * 
 * Copyright (C) Damian Gareth Walker 2020.
 * Created: 22-Jun-2020.
 */

#include <stdio.h>
#include <stdlib.h>
#include <graph.h>

/*----------------------------------------------------------------------
 * Public Level Functions.
 */

/**
 * Exit the program on an error condition, showing the error.
 * @param exit_code is the code that determines what the error was.
 */
void exit_game (int exit_code)
{
    /* @var message is a pointer to the output message */
    char *message;

    /* determine what message to print */
    switch (exit_code) {
        case 0:
            message = "Goodbye!";
            break;
        case 1:
            message = "Cannot initialise screen!";
            break;
        case 2:
            message = "Cannot load font!";
            break;
        case 3:
            message = "Cannot load graphics!";
            break;
        case 4:
            message = "Invalid option!";
            break;
        case 5:
            message = "This beta has expired! Contact Cyningstan.";
            break;
        default:
            message = "Unknown error!";
    }

    /* make sure we're in text mode and print the message */
    _setvideomode (_DEFAULTMODE);
    puts (message);
    exit (exit_code);
}
