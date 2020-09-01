/*----------------------------------------------------------------------
 * Ossuary:
 * A DOS port of the ZX Spectrum rogue-like.
 * Game handling module.
 * 
 * Copyright (C) Damian Gareth Walker 2013, 2020.
 * Created: 22-Jun-2020.
 */

/* compiler headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <dos.h>

/* project headers */
#include "game.h"
#include "display.h"

/*----------------------------------------------------------------------
 * Constant definitions.
 */

/* 3x3 cell map bitmasks */
#define CELL_EAST    0x01
#define CELL_SOUTH   0x02
#define CELL_WEST    0x04
#define CELL_NORTH   0x08
#define CELL_CHAMBER 0x80

/* bits in the quick inventory */
#define QI_WAND 0x01
#define QI_AMULET 0x02
#define QI_WOODEN_SHIELD 0x04
#define QI_IRON_SHIELD 0x08
#define QI_DAGGER 0x10
#define QI_SWORD 0x20
#define QI_AXE 0x40
#define QI_POTION 0x80

/* composite bit patterns for easy searches in the quick inventory */
#define QI_ANY_SHIELD 0x0c
#define QI_BIG_WEAPON 0x60
#define QI_ANY_WEAPON 0x70

/*----------------------------------------------------------------------
 * Type definitions
 */

/** @enum Behaviour is a list of enemy behaviours */
typedef enum {
    BEHAVIOUR_GUARD,
    BEHAVIOUR_WANDER,
    BEHAVIOUR_CHASE
} Behaviour;

/** @enum EnemyType is a list of enemy types */
typedef enum {
    ENEMY_BAT,
    ENEMY_VERMIN,
    ENEMY_ARACHNID,
    ENEMY_SERPENT,
    ENEMY_CADAVER,
    ENEMY_SKELETON,
    ENEMY_SPECTRE,
    ENEMY_DAEMON
} EnemyType;

/** @enum ItemType is a list of item types */
typedef enum {
    ITEM_NONE,
    ITEM_PURSE,
    ITEM_CASKET,
    ITEM_APPLE,
    ITEM_POTION,
    ITEM_DAGGER,
    ITEM_SWORD,
    ITEM_AXE,
    ITEM_WOODEN_SHIELD,
    ITEM_IRON_SHIELD,
    ITEM_SCROLL,
    ITEM_AMULET,
    ITEM_WAND
} ItemType;

/** @enum Direction is a list of directions of movement */
typedef enum {
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

/** @enum MenuOptions is a list of main menu options */
typedef enum {
    MENU_HELP,
    MENU_WAIT,
    MENU_INVENTORY,
    MENU_PICK_UP,
    MENU_DESCEND,
    MENU_EAT,
    MENU_QUAFF,
    MENU_NEW_GAME,
    MENU_EXIT,
    MENU_CANCEL,
    MENU_INV_HELP,
    MENU_INV_WIELD,
    MENU_INV_PUT_AWAY,
    MENU_INV_CONSUME,
    MENU_INV_DISCARD,
    MENU_INV_CLOSE
} MenuOptions;

/** @struct enemy is an enemy record */
typedef struct enemy Enemy;
struct enemy {

    /** @var x is the x location of the enemy */
    int x;

    /** @var y is the y location of the enemy */
    int y;

    /** @var type is the type of enemy */
    int type;

    /** @var health is the enemy's health */
    int health;

    /** @var aggression is the aggression level of the enemy */
    int aggression;

    /** @var behaviour indicates what the enemy is doing */
    Behaviour behaviour;

    /** @var dest_x is the X coordinate the monster is heading for */
    int dest_x;

    /** @var dest_y is the Y coordinate the monster is heading for */
    int dest_y;
};

/** @struct enemy_stats are the stats of each enemy type */
typedef struct enemy_stats EnemyStats;
struct enemy_stats {

    /** @var strength is the enemy strength */
    int strength;

    /** @var agility is the enemy agility */
    int agility;
};

/*----------------------------------------------------------------------
 * File level variables.
 */

/** @var name is the player's name */
static char name[11];

/** @var level is the current dungeon level. */
static int level;

/** @var gold is the gold that the hero has collected. */
static int gold;

/** @var enemy_min is the weakest enemy for the current level. */
static int enemy_min;

/** @var enemy_max is the strongest enemy for the current level. */
static int enemy_max;

/** @var enemy_anger is the average enemy aggression level. */
static int enemy_anger;

/** @var experience is the number of XP points the player has. */
static int experience = 0;
 
/** @var necro_health is the necromancer's hit points. */
static int necro_health;

/** @var necro_x is the necromancer's X position */
static int necro_x;

/** @var necro_y is the necromancer's Y position */
static int necro_y;

/** @var hero_strength is the primary attack stat. */
static int hero_strength;

/** @var hero_agility is the primary defence stat. */
static int hero_agility;

/** @var hero_intelligence is the magical attack/defence stat. */
static int hero_intelligence;

/** @var hero_x is the hero's X position on the map */
static int hero_x;

/** @var hero_y is the hero's Y position on the map */
static int hero_y;

/** @var hero_health is the hero's hit points. */
static int hero_health;

/** @var inventory is the player's inventory */
static int inventory[12];

/** @var inventory_slot is the last inventory slot accessed. */
static int inventory_slot;

/** @var strength_mod is the attack modifier */
static int strength_modifier;

/** @var agility_mod is the defence modifier */
static int agility_modifier;

/** @var intelligence_mod is the intelligence modifier */
static int intelligence_modifier;

/** @var map is the map */
static char map[16][16];

/** @var enemies is the array of enemies */
static Enemy enemies[8];

/** @var level_over signifies that the level is over */
static int level_over;

/** @var game_over signifies that the user wants to restart */
static int game_over;

/** @var game_quit signifies that the user has quit */
static int game_quit;

/** @var enemy_stats is an array of stats per enemy type */
static EnemyStats enemy_stats[8] = {
    {2, 6},
    {6, 4},
    {6, 6},
    {8, 6},
    {8, 8},
    {8, 10},
    {10, 10},
    {10, 12}
};

/** @var item_message are shown when standing by each item */
static char item_message[16][17] = {
    "  FOUND A BUG!  ",
    "A PURSE OF GOLD ",
    "A CASKET OF GOLD",
    "  A NICE APPLE  ",
    "A HEALING POTION",
    " A SMALL DAGGER ",
    "  A BROADSWORD  ",
    "  A BATTLE AXE  ",
    "A WOODEN SHIELD ",
    " AN IRON SHIELD ",
    " A BANISH SPELL ",
    " A MAGIC AMULET ",
    "  A MAGIC WAND  ",
    "  FOUND A BUG!  ",
    "  FOUND A BUG!  ",
    "THE STAIRS DOWN "
};

/** @var enemy_names are the names of the enemy types */
static char enemy_names[8][9] = {
    "BAT",
    "VERMIN",
    "ARACHNID",
    "SERPENT",
    "CADAVER",
    "SKELETON",
    "SPECTRE",
    "DAEMON"
};

/** @var menu_option_text is the text to print for main menu options */
static char menu_option_text[16][17] = {
    " SHOW GAME KEYS ",
    "WAIT A MOMENT...",
    " VIEW INVENTORY ",
    "  PICK UP ITEM  ",
    " DESCEND STAIRS ",
    "EAT A NICE APPLE",
    " QUAFF A POTION ",
    " START NEW GAME ",
    " EXIT THIS GAME ",
    "  CANCEL MENU   ",
    " SHOW GAME KEYS ",
    "   WIELD ITEM   ",
    " PUT ITEM AWAY  ",
    "  CONSUME ITEM  ",
    "  DISCARD ITEM  ",
    "CLOSE INVENTORY "
};

/*----------------------------------------------------------------------
 * Service Routines.
 */

/**
 * Roll a number of six-sided dice.
 * @param count is the number of dice to roll.
 * @returns the total of the dice.
 */
static int dice (int count)
{
    int total = 0;
    while (count--)
        total += 1 + rand() % 6;
    return total;
}

/*----------------------------------------------------------------------
 * Level 4 Routines.
 */

/**
 * Make one of the enemies attack the hero.
 * @param enemy_no is the enemy who's attacking the hero.
 * @returns 1 if the hero is killed, 0 if not.
 */
static int attack_hero (int enemy_no)
{
    /* local variables */
    int attack, /* enemy's attack value */
        defence, /* hero's defence value */
        damage, /* damage caused */
        update; /* 1 if the screen needs updating */
    char message[17]; /* battle round message */
    Enemy *enemy; /* enemy the hero is attacking */

    /* determine which enemy and make it more aggressive */
    enemy = &enemies[enemy_no];

    /* determine the enemy's attack value */
    attack = enemy_stats[enemy->type].strength;

    /* determine hero's defence value */
    if (inventory[0] == ITEM_AMULET || inventory[1] == ITEM_AMULET)
        defence = hero_intelligence + intelligence_modifier;
    else
        defence = hero_agility + agility_modifier;
    
    /* otherwise conduct an ordinary attack */
    damage = (1 + attack + dice (2)) - (defence + dice (2));
    if (damage < 0)
        damage = 0;
    
    /* register the damage */
    hero_health -= damage;
    if (hero_health < 0)
        hero_health = 0;
    update = 0;

    /* process a miss */
    if (! damage) {
        sprintf (message, "%-8s MISSES ", enemy_names[enemy->type]);
        display_message (message);
    }

    /* process a non-fatal hit */
    else if (hero_health) {
        sprintf (message, "%-8s HIT YOU", enemy_names[enemy->type]);
        display_message (message);
        display_health (hero_health, hero_strength + hero_agility
            + hero_intelligence);
        hit_noise ();
    }

    /* process a fatal hit */
    else {
        sprintf (message, "%-8s HIT YOU", enemy_names[enemy->type]);
        display_message (message);
        display_health (hero_health, hero_strength + hero_agility
            + hero_intelligence);
        kill_noise ();
        map[hero_x][hero_y] = (map[hero_x][hero_y] & 0xf) | 0x10;
        update_map_tile (hero_x, hero_y, map[hero_x][hero_y]);
        update = 1;
        game_over = 1;
    }

    /* return 1 if screen update required */
    return update;
}

/**
 * Kill an enemy.
 * Remove it from the map.
 * Process any experience level advancements.
 * @param enemy is a pointer to the enemy killed.
 * @param xp_award is the xp awarded.
 */
static void kill_enemy (Enemy *enemy, int xp_award)
{
    /* local variables */
    int stat_no; /* random player stat to increase */

    /* remove the enemy from the map */
    map[enemy->x][enemy->y] = (map[enemy->x][enemy->y] & 0xf) | 0x10;
    update_map_tile (enemy->x, enemy->y, map[enemy->x][enemy->y]);
    if (! xp_award) return;

    /* process advancement */
    experience += xp_award;
    while (experience >= 24) {
        stat_no = rand () % 3;
        if (stat_no == 0 && hero_strength < 12) {
            ++hero_strength;
            display_stat (0, hero_strength, strength_modifier);
            experience = 0;
        } else if (stat_no == 1 && hero_agility < 12) {
            ++hero_agility;
            display_stat (1, hero_agility, agility_modifier);
            experience = 0;
        } else if (stat_no == 2 && hero_intelligence < 12) {
            ++hero_intelligence;
            display_stat (2, hero_intelligence,
                intelligence_modifier);
            experience -= 24;
        }
    }
    if (experience == 0) {
        ++hero_health;
        display_health (hero_health, hero_strength + hero_agility
            + hero_intelligence);
    }
}

/*----------------------------------------------------------------------
 * Level 3 Routines.
 */
 
/**
 * Attempt to use a banish spell.
 * @param enemy_x is the X coordinate of the enemy.
 * @param enemy_y is the Y coordinate of the enemy.
 * @returns 1 if the screen needs updating.
 */
static int banish_spell (int enemy_x, int enemy_y)
{
    /* local variables */
    int roll, /* the roll of two dice */
        update = 0, /* 1 if the screen map needs updating */
        scroll_slot, /* which inventory slot the scroll is in */
        e; /* enemy counter */
        
    /* the effect is decided by a roll of 2D6 */
    roll = dice (2);
    
    /* on 8+, banish the monster */
    if (roll >= 8) {
        for (e = 0; e < 8; ++e)
            if (enemies[e].x == enemy_x && enemies[e].y == enemy_y) {
                enemies[e].health = 0;
                kill_enemy (&enemies[e], 0);
            }
        display_message ("BANISH SUCCEEDED");
        update = 1;
        kill_noise ();
    }
    
    /* otherwise on 4+, the spell simply fails */
    else if (roll >= 4)
        display_message (" BANISH FAILED  ");
        
    /* finally on a roll of 3-, the spell disappears! */
    else {
        scroll_slot = (inventory[1] == ITEM_SCROLL);
        inventory[scroll_slot] = 0;
        display_inventory (scroll_slot, 0);
        display_message ("SCROLL HAS GONE!");
    }
    
    /* tell the calling process whether the map needs updating */
    return update;
}

/**
 * Recalculate and display modifiers.
 */
static void recalc_modifiers (void)
{
    /* local variables */
    int c; /* inventory counter */

    /* initialise modifiers */
    strength_modifier = agility_modifier = intelligence_modifier = 0;
    
    /* look at wielded items */
    for (c = 0; c < 2; ++c)
        switch (inventory[c]) {
            case ITEM_DAGGER: strength_modifier += 2; break;
            case ITEM_SWORD: strength_modifier += 4; break;
            case ITEM_AXE: strength_modifier += 6; break;
            case ITEM_WOODEN_SHIELD: agility_modifier += 2; break;
            case ITEM_IRON_SHIELD: agility_modifier += 4; break;
            case ITEM_WAND: intelligence_modifier += 2; break;
            case ITEM_AMULET: intelligence_modifier += 2; break;
        }
    
    /* display the stats and modifiers */
    display_stat (0, hero_strength, strength_modifier);
    display_stat (1, hero_agility, agility_modifier);
    display_stat (2, hero_intelligence, intelligence_modifier);
}

/**
 * Attempt to move an enemy.
 * @param enemy_no is the enemy number.
 * @param x is the X coordinate to move to.
 * @param y is the Y coorddinate to move to.
 * @returns 1 if successful, 0 if blocked.
 */
static int move_enemy (int enemy_no, int x, int y)
{
    /* local variables */
    int moved = 0, /* set to 1 if moved */
        cx, /* enemy's current X position */
        cy, /* enemy's current Y position */
        item; /* item the enemy is carrying or guarding */

    /* the hero is here - attack! */
    if ((map[x][y] & 0xf0) == 0x20) {
        attack_hero (enemy_no);
        moved = 1;
    }
    
    /* the square is empty - move there */
    else if ((map[x][y] & 0xf0) == 0x10) {

        /* record enemy's current position in convenience variables */
        cx = enemies[enemy_no].x;
        cy = enemies[enemy_no].y;
        item = map[cx][cy] & 0xf;

        /* first move the enemy on the map */
        map[x][y] = map[x][y]
            & 0x0f
            | 0x80
            | (enemies[enemy_no].type << 4);
        map[cx][cy] = map[cx][cy] & 0x0f | 0x10;

        /* then bring any item with it, if appropriate */
        if ((map[x][y] & 0xf) == 0
            && item > 0
            && item < 0xf
            && (enemies[enemy_no].type == ENEMY_BAT
            || (enemies[enemy_no].type >= ENEMY_CADAVER
            && enemies[enemy_no].type != ENEMY_SPECTRE))) {
            map[x][y] |= item;
            map[cx][cy] &= 0xf0;
        }

        /* update the map and enemy data */
        update_map_tile (x, y, map[x][y]);
        update_map_tile (cx, cy, map[cx][cy]);
        enemies[enemy_no].x = x;
        enemies[enemy_no].y = y;
        moved = 1;
    }

    /* tell the calling function if the move was successful */
    return moved;
}

/*----------------------------------------------------------------------
 * Level 2 Routines.
 */

/**
 * Attack an enemy on a particular square
 * @param enemy_x is the X coordinate to attack.
 * @param enemy_y is the Y coordinate to attack.
 * @returns 1 if the enemy is killed, 0 if not.
 */
static int attack_enemy (int enemy_x, int enemy_y)
{
    /* local variables */
    int c, /* enemy counter */
        attack, /* hero's attack value */
        defence, /* enemy's defence value */
        damage, /* damage caused */
        update; /* 1 if the screen needs updating */
    char message[17]; /* battle round message */
    Enemy *enemy; /* enemy the hero is attacking */

    /* determine which enemy and make it more aggressive */
    enemy = NULL;
    for (c = 0; c < 8; ++c)
        if (enemies[c].x == enemy_x
            && enemies[c].y == enemy_y
            && enemies[c].health > 0)
            enemy = &enemies[c];
    enemy->aggression += (enemy->aggression < 0xff);

    /* determine hero's attack value */
    if (inventory[0] == ITEM_WAND || inventory[1] == ITEM_WAND)
        attack = hero_intelligence + intelligence_modifier;
    else if (enemy->type <= ENEMY_SKELETON)
        attack = hero_strength + strength_modifier;
    else
        attack = 0;

    /* use banish spell where appropriate */
    if (enemy->type >= ENEMY_SPECTRE
        && (inventory[0] == ITEM_SCROLL
        || inventory[1] == ITEM_SCROLL)) {
        return banish_spell (enemy_x, enemy_y);
    }
    
    /* otherwise conduct an ordinary attack */
    else if (attack > 0) {
        defence = enemy_stats[enemy->type].agility;
        damage = (1 + attack + dice (2)) - (defence + dice (2));
        if (damage < 0)
            damage = 0;
    }
    
    /* if the enemy can't be attacked, register a miss */
    else
        damage = 0;

    /* register the damage */
    enemy->health -= damage;
    if (enemy->health < 0)
        enemy->health = 0;
    update = 0;

    /* process a miss */
    if (! damage) {
        sprintf (message, " MISSED %-8s", enemy_names[enemy->type]);
        display_message (message);
    }

    /* process a non-fatal hit */
    else if (enemy->health) {
        sprintf (message, "YOU HIT %-8s", enemy_names[enemy->type]);
        display_message (message);
        hit_noise ();
    }

    /* process a fatal hit */
    else {
        kill_enemy (enemy, 1);
        sprintf (message, " KILLED %-8s", enemy_names[enemy->type]);
        display_message (message);
        update = 1;
        kill_noise ();
    }

    /* return 1 if screen update required */
    return update;
}

/**
 * Attack the necromancer.
 * @returns 1 if the enemy is killed, 0 if not.
 */
static int attack_necromancer (void)
{
    /* local variables */
    int attack, /* hero's attack value */
        defence, /* enemy's defence value */
        damage, /* damage caused */
        update; /* 1 if the screen needs updating */

    /* determine hero's attack value */
    if (inventory[0] == ITEM_WAND || inventory[1] == ITEM_WAND)
        attack = hero_intelligence + intelligence_modifier;
    else
        attack = hero_strength + strength_modifier;

    /* conduct the attack */
    defence = 12;
    damage = (1 + attack + dice (2)) - (defence + dice (2));
    if (damage < 0)
        damage = 0;

    /* register the damage */
    necro_health -= damage;
    if (necro_health < 0)
        necro_health = 0;
    update = 0;

    /* process a miss */
    if (! damage)
        display_message (" MISSED SORCEROR");

    /* process a non-fatal hit */
    else if (necro_health) {
        display_message ("YOU HIT SORCEROR");
        hit_noise ();
    }

    /* process a fatal hit */
    else {
        update_map_tile (necro_x, necro_y, 0x10);
        display_map (hero_x, hero_y);
        display_message ("SORCEROR IS GONE");
        kill_noise ();
        update = 1;
    }

    /* return 1 if screen update required */
    return update;
}

/**
 * Show the inventory menu and get an option.
 * @return the option chosen.
 */
static int inventory_menu (void)
{
    /* local variables */
    int option, /* option currently selected */
        finished, /* finished selecting option */
        key; /* key press */
    
    /* decide on the default option */
    if (inventory[inventory_slot] == 0)
        option = MENU_INV_CLOSE;
    else if (inventory[inventory_slot] == ITEM_APPLE
        || inventory[inventory_slot] == ITEM_POTION)
        option = MENU_INV_CONSUME;
    else if (inventory_slot < 2)
        option = MENU_INV_PUT_AWAY;
    else
        option = MENU_INV_WIELD;

    /* main menu loop */
    finished = 0;
    while (! finished) {
    
        /* display the current option and await a keypress */
        display_message (menu_option_text[option]);
        key = get_keypress ();
        
        /* see if the user has finished */
        if (key == 13 || key == 32 || key == 27)
            finished = 1;

        /* cycle through the menu options */
        else if (key == KEY_UP || key == KEY_KEYPAD_UP
            || key == KEY_LEFT || key == KEY_KEYPAD_LEFT)
            --option;
        else if (key == KEY_DOWN || key == KEY_KEYPAD_DOWN
            || key == KEY_RIGHT || key == KEY_KEYPAD_RIGHT)
            ++option;

        /* choose a menu option based on a shortcut */
        else if (key == '?' || key == 'h') {
            option = MENU_INV_HELP;
            finished = 1;
        } else if (key == 'w') {
            option = MENU_INV_WIELD;
            finished = 1;
        } else if (key == 'p') {
            option = MENU_INV_PUT_AWAY;
            finished = 1;
        } else if (key == 'c') {
            option = MENU_INV_CONSUME;
            finished = 1;
        } else if (key == 'e' && inventory[inventory_slot] == ITEM_APPLE) {
            option = MENU_INV_CONSUME;
            finished = 1;
        } else if (key == 'q'
            && inventory[inventory_slot] == ITEM_POTION) {
            option = MENU_INV_CONSUME;
            finished = 1;
        } else if (key == 'd') {
            option = MENU_INV_DISCARD;
            finished = 1;
        }

        /* make sure we're pointing at a valid menu option */
        if (option < MENU_INV_HELP)
            option = MENU_INV_CLOSE;
        else if (option > MENU_INV_CLOSE)
            option = MENU_INV_HELP;
    }
    blank_message ();
    
    /* return the option chosen */
    return (key == 27) ? MENU_INV_CLOSE : option;
}

/**
 * Show a list of keys
 */
static void show_keys (void)
{
    display_keys ();
    show_level ();
    display_map (hero_x, hero_y);
}

/**
 * Wield the item highlighted by the inventory cursor.
 * @returns 1 on success.
 */
static int wield_item (void)
{
    /* local variables */
    int empty_slot, /* empty slot identified in hand */
        success; /* 1 if an item was wielded */
    
    /* validation */
    success = 0;
    if (inventory_slot < 2)
        display_message ("ALREADY WIELDED!");
    else if (inventory[0] && inventory[1])
        display_message (" HANDS ARE FULL ");
    else if (inventory[inventory_slot] == 0)
        display_message ("NOTHING TO WIELD");

    /* wield the item */
    else {
        display_message ("ITEM NOW WIELDED");
        empty_slot = (inventory[0] != 0);
        inventory[empty_slot] = inventory[inventory_slot];
        inventory[inventory_slot] = 0;
        display_inventory (empty_slot, inventory[empty_slot]);
        display_inventory (inventory_slot, inventory[inventory_slot]);
        inventory_slot = empty_slot;
        recalc_modifiers ();
        success = 1;
    }
    
    /* return a success code */
    return success;
}

/**
 * Put the highlighted wielded item in the bag.
 * @returns 1 on success.
 */
static int put_item_away (void)
{
    /* local variables */
    int empty_slot, /* empty slot identified in hand */
        success, /* 1 if an item was wielded */
        c; /* inventory slot counter */
    
    /* validation */
    success = 0;
    if (inventory_slot > 1)
        display_message ("ITEM NOT IN HAND");
    else if (inventory[inventory_slot] == 0)
        display_message ("ITEM NOT IN HAND");

    /* wield the item */
    else {
        empty_slot = -1;
        for (c = 2; c < 12; ++c)
            if (inventory[c] == 0)
                empty_slot = c;
        if (empty_slot == -1)
            display_message ("YOUR BAG IS FULL");
        else {
            display_message ("ITEM IS PUT AWAY");
            inventory[empty_slot] = inventory[inventory_slot];
            inventory[inventory_slot] = 0;
            display_inventory (empty_slot, inventory[empty_slot]);
            display_inventory (inventory_slot, inventory[inventory_slot]);
            inventory_slot = empty_slot;
            recalc_modifiers ();
            success = 1;
        }
    }

    /* return a success code */
    return success;
}

/**
 * Consume the highlighted item.
 * @returns 1 on success.
 */
static int consume_item (void)
{
    /* local variables */
    int success, /* 1 if item successfully consumed */
        max_health; /* convenience variable */

    /* initialise */
    success = 0;
    max_health = hero_strength + hero_agility + hero_intelligence;
    
    /* don't waste food if already healthy */
    if (hero_health == max_health)
        display_message ("ALREADY HEALTHY ");

    /* eat the delicious apple */
    else if (inventory[inventory_slot] == 3) {
        hero_health += 4;
        if (hero_health > max_health)
            hero_health = max_health;
        display_message (" ATE THE APPLE  ");
        display_health (hero_health, max_health);
        inventory[inventory_slot] = 0;
        display_inventory (inventory_slot, 0);
        success = 1;
    }
    
    /* drink (potion) and be merry */
    else if (inventory[inventory_slot] == 4) {
        hero_health = max_health;
        display_message (" QUAFFED POTION ");
        display_health (hero_health, max_health);
        inventory[inventory_slot] = 0;
        display_inventory (inventory_slot, 0);
        success = 1;
    }
    
    /* reject inedible items */
    else
        display_message (" NO EDIBLE ITEM ");

    /* return a success code */
    return success;
}

/**
 * Discard the item highlighted by the inventory cursor.
 * @returns 1 on success.
 */
static int discard_item ()
{
    /* local variables */
    int success; /* 1 if item successfully dropped */
    
    /* make sure there's somewhere to drop the item */
    success = 0;
    if (! inventory[inventory_slot])
        display_message ("NOTHING TO DROP ");
    else if ((map[hero_x][hero_y] & 0xf) != 0)
        display_message ("CANNOT DROP HERE");

    /* discard the item */
    else {
        display_message (" ITEM DISCARDED ");
        map[hero_x][hero_y] |= inventory[inventory_slot];
        inventory[inventory_slot] = 0;
        display_inventory (inventory_slot, 0);
        if (inventory_slot < 2)
            recalc_modifiers ();
        success = 1;
    }
    
    /* return success */
    return success;
}

/**
 * Pathfind for an enemy.
 * @param enemy_no is the enemy number.
 * @param x is the X coordinate to pathfind to.
 * @param y is the Y coordinate to pathfind to.
 * @param chasing is 1 if the enemy is chasing the hero.
 */
static void enemy_pathfind (int enemy_no, int x, int y, int chasing)
{
    /* local variables */
    int xd, /* X direction */
        yd, /* Y direction */
        block_x, /* X coordinate of adjacent block */
        block_y, /* Y coordinate of adjacent block */
        order, /* randomised order to consider directions */
        axis; /* axis under consideration */

    /* ascertain directions to head for and order of axes */
    xd = (x > enemies[enemy_no].x) - (x < enemies[enemy_no].x);
    yd = (y > enemies[enemy_no].y) - (y < enemies[enemy_no].y);
    order = rand () % 2;
    
    /* attempt to move in each direction */
    for (axis = 0; axis <= 1; ++axis) {
    
        /* look at an adjacent block */
        block_x = enemies[enemy_no].x;
        block_y = enemies[enemy_no].y;
        if (axis ^ order)
            block_y += yd;
        else
            block_x += xd;

        /* attempt to move the enemy in this direction */
        if ((block_x != enemies[enemy_no].x
            || block_y != enemies[enemy_no].y)
            && move_enemy (enemy_no, block_x, block_y))
            return;
    }
    
    /* we didn't manage to move... what to do? */
    if (! chasing) {
        enemies[enemy_no].dest_x = rand () % 16;
        enemies[enemy_no].dest_y = rand () % 16;
    }
}

/*----------------------------------------------------------------------
 * Level 1 Routines.
 */

/**
 * Scramble the cell numbers.
 * This provides an easy way to link up all the cells randomly.
 * @param cell_order is a scrambled index to the cells.
 */
static void scramble_cells (int *cell_order)
{
    /* local variables */
    int cell_count, /* cell counter */
        cell_swap; /* cell to swap with */
    
    /* loop through the cell index swapping at random */
    for (cell_count = 0; cell_count < 9; ++cell_count) {
        do {
            cell_swap = rand () % 9;
        } while (cell_swap == cell_count);
        cell_order[cell_swap] ^= cell_order[cell_count];
        cell_order[cell_count] ^= cell_order[cell_swap];
        cell_order[cell_swap] ^= cell_order[cell_count];
    }
}

/**
 * Generate a network of paths between the 3x cells.
 * Since there's a randomised cell index, beating a path from 0->1,
 * 1->2 .. 7->8 in the cardinal directions generates a nice 3x3 dungeon.
 * @param cell_order is a scrambled index to the cells.
 * @param cell_map is the 3x3 cell map.
 */
static void generate_paths (int *cell_order, int *cell_map)
{
    /* local variables */
    int cell_count, /* cell counter */
        curr_cell, /* cell to dig a path from */
        dest_cell; /* cell to dig a path towards */

    /* block off any paths from the previous level */
    for (cell_count = 0; cell_count < 9; ++cell_count)
        cell_map[cell_count] = 0;
    
    /* create the 8 paths between the 9 randomised cells */
    for (cell_count = 0; cell_count < 8; ++cell_count) {

        /* ascertain the two cells we're connecting */
        curr_cell = cell_order[cell_count];
        dest_cell = cell_order[cell_count + 1];

        /* dig a vertical path */
        while (curr_cell / 3 != dest_cell / 3) {
            if (curr_cell < dest_cell) {
                cell_map[curr_cell] |= CELL_SOUTH;
                cell_map[curr_cell + 3] |= CELL_NORTH;
                curr_cell += 3;
            } else {
                cell_map[curr_cell] |= CELL_NORTH;
                cell_map[curr_cell - 3] |= CELL_SOUTH;
                curr_cell -= 3;
            }
        }

        /* dig a horizontal path */
        while (curr_cell != dest_cell) {
            if (curr_cell < dest_cell) {
                cell_map[curr_cell] |= CELL_EAST;
                cell_map[++curr_cell] |= CELL_WEST;
            } else {
                cell_map[curr_cell] |= CELL_WEST;
                cell_map[--curr_cell] |= CELL_EAST;
            }
        }
    }
}

/**
 * Generate the chambers.
 * There's a 50% chance that each of the cells on the 3x3 grid will be
 * a chamber, otherwise it will remain a corridor. Dead ends are also
 * turned into chambers, so the proportion of chambers to corridors will
 * be more than 50:50.
 * @params cell_map is the cell map to modify.
 */
static void generate_chambers (int *cell_map)
{
    /* local variables */
    int cell_count; /* counter for looping through cells */

    /* loop through the cells */
    for (cell_count = 0; cell_count < 9; ++cell_count) {

        /* dead ends are always chambers */
        if (cell_map[cell_count] == CELL_EAST ||
            cell_map[cell_count] == CELL_SOUTH ||
            cell_map[cell_count] == CELL_WEST ||
            cell_map[cell_count] == CELL_NORTH)
            cell_map[cell_count] |= CELL_CHAMBER;
        
        /* the rest are a 50/50 chance */
        else if (rand () % 2)
            cell_map[cell_count] |= CELL_CHAMBER;
    }
}

/**
 * fill the 16x16 map with wall.
 */
static void fill_map ()
{
    memset (map, 0, 256);
}

/**
 * Dig out the room and corridor cells on the 16x16 map.
 * @params cell_map is the 3x3 map of cells.
 */
static void open_cells (int *cell_map)
{
    /* local variables */
    int cell_count; /* counter for looping through cells */
    int x, y; /* map square pointer */
    int offset; /* offset count for digging corridors */

    /* loop through the cells */
    for (cell_count = 0; cell_count < 9; ++cell_count) {

        /* calculate x and y coordinates of the top left corner wall */
        x = 5 * (cell_count % 3);
        y = 5 * (cell_count / 3);

        /* open out a chamber */
        if (cell_map[cell_count] & CELL_CHAMBER) {

            /* open up chamber exits */
            if (cell_map[cell_count] & CELL_SOUTH)
                map[x + 2][y + 5] = 0x10;
            if (cell_map[cell_count] & CELL_EAST)
                map[x + 5][y + 2] = 0x10;

            /* dig out chamber */
            for (x = 1 + 5 * (cell_count % 3);
                x < 5 + 5 * (cell_count % 3);
                ++x)
                for (y = 1 + 5 * (cell_count / 3);
                    y < 5 + 5 * (cell_count / 3);
                    ++y)
                    map[x][y] = 0x10;
        }
        
        /* open up a corridor */
        else {
            for (offset = 0; offset <= 5; ++offset) {
                if (offset <= 2 && (cell_map[cell_count] & CELL_NORTH))
                    map[x + 2][y + offset] = 0x10;
                if (offset >= 2 && (cell_map[cell_count] & CELL_SOUTH))
                    map[x + 2][y + offset] = 0x10;
                if (offset <= 2 && (cell_map[cell_count] & CELL_WEST))
                    map[x + offset][y + 2] = 0x10;
                if (offset >= 2 && (cell_map[cell_count] & CELL_EAST))
                    map[x + offset][y + 2] = 0x10;
            }
        }
    }
}

/**
 * Decorate some exposed walls with gargoyles, niches and torches.
 */
static void decorate_walls ()
{
    /* local variables */
    int x, y; /* coordinate counters */

    /* loop through all map squares looking for exposed walls */
    for (x = 0; x < 15; ++x)
        for (y = 0; y < 15; ++y)
            if (map[x][y] == 0
                && (x > 0 && map[x - 1][y] == 0x10
                    || x < 15 && map[x + 1][y] == 0x10
                    || y > 0 && map[x][y - 1] == 0x10
                    || y < 15 && map[x][y + 1] == 0x10)
                && rand () % 2)
                map[x][y] = rand () & 3;
}

/**
 * Build a quick list of key items in the player's inventory.
 * @returns a bit-field collection of flags for various items.
 */
static int get_quick_inventory (void)
{
    /* local variables */
    int quick_inventory, /* the inventory to return */
        i_count; /* a counter to loop through items */

    /* see what the player has */
    quick_inventory = 0;
    for (i_count = 0; i_count < 12; ++i_count)
        switch (inventory[i_count]) {
            case 12: quick_inventory |= QI_WAND; break;
            case 11: quick_inventory |= QI_AMULET; break;
            case 9: quick_inventory |= QI_IRON_SHIELD; break;
            case 8: quick_inventory |= QI_WOODEN_SHIELD; break;
            case 7: quick_inventory |= QI_AXE; break;
            case 6: quick_inventory |= QI_SWORD; break;
            case 5: quick_inventory |= QI_DAGGER; break;
            case 4: quick_inventory |= QI_POTION; break;
        }
    
    /* return the quick inventory */
    return quick_inventory;
}

/**
 * Build up a list of items to place on the current level.
 * @param items_list is the array of items to build.
 * @param quick_inventory is a bit-encoded list of what the player has.
 * The quick_inventory parameter is also abused to mark off items as we
 * add them to item_list.
 */
static void build_items_list (int *items_list, int quick_inventory)
{
    /* local variables */
    int i_count, /* count of items */
        added_potions, /* number of potions added */
        added_scroll, /* 1 if a scroll already added */
        apples_to_add; /* random number of apples for the level */

    /* initialise counters */
    added_potions = added_scroll = 0;
    apples_to_add = rand () % 4;

    /* build the six non-gold items on the level*/
    for (i_count = 0; i_count < 6; ++i_count)

        /* consider adding a wand */
        if (! (quick_inventory & QI_WAND)) {
            items_list[i_count] = ITEM_WAND;
            quick_inventory |= QI_WAND;
        }

        /* consider adding an amulet */
        else if (enemy_max >= ENEMY_CADAVER
            && ! (quick_inventory & QI_AMULET)) {
            items_list[i_count] = ITEM_AMULET;
            quick_inventory |= QI_AMULET;
        }

        /* consider adding a scroll */
        else if (enemy_max >= ENEMY_SKELETON && ! added_scroll) {
            items_list[i_count] = ITEM_SCROLL;
            ++added_scroll;
        }

        /* consider adding an iron shield */
        else if (enemy_max >= ENEMY_CADAVER
            && (! (quick_inventory & QI_IRON_SHIELD))) {
            items_list[i_count] = ITEM_IRON_SHIELD;
            quick_inventory |= QI_IRON_SHIELD;
        }

        /* consider adding a wooden shield */
        else if (enemy_max >= ENEMY_ARACHNID
            && ! (quick_inventory & QI_ANY_SHIELD)) {
            items_list[i_count] = ITEM_WOODEN_SHIELD;
            quick_inventory |= QI_WOODEN_SHIELD;
        }

        /* consider adding an axe */
        else if (enemy_max >= ENEMY_SKELETON
            && ! (quick_inventory & QI_AXE)) {
            items_list[i_count] = ITEM_AXE;
            quick_inventory |= QI_AXE;
        }

        /* consider adding a sword */
        else if (enemy_max >= ENEMY_SERPENT
            && ! (quick_inventory & QI_BIG_WEAPON)) {
            items_list[i_count] = ITEM_SWORD;
            quick_inventory |= QI_SWORD;
        }

        /* consider adding a dagger */
        else if (! (quick_inventory & QI_ANY_WEAPON)) {
            items_list[i_count] = ITEM_DAGGER;
            quick_inventory |= QI_DAGGER;
        }

        /* add the first potion */
        else if (! added_potions) {
            items_list[i_count] = ITEM_POTION;
            ++added_potions;
        }

        /* consider adding a second potion */
        else if (enemy_max == ENEMY_DAEMON && added_potions < 2) {
            items_list[i_count] = ITEM_POTION;
            ++added_potions;
        }

        /* add the appropriate number of apples */
        else if (apples_to_add-- > 0)
            items_list[i_count] = ITEM_APPLE;

        /* fill the rest of the slots with gold */
        else
            items_list[i_count]
                = 1 + (rand () % 24) >= level
                ? ITEM_PURSE
                : ITEM_CASKET;
}

/**
 * Place onto the map the items chosen.
 * @param items_list is the list of items chosen by build_items_list().
 */
static void place_listed_items (int *items_list)
{
    /* local variables */
    int i_count, /* a counter of the items */
        x, y; /* temporary holders for location */

    /* loop through the items */
    for (i_count = 0; i_count < 6; ++i_count) {

        /* the first four items are guarded */
        if (i_count < 4) {
            x = enemies[i_count].x;
            y = enemies[i_count].y;
            if ((enemies[i_count].type >= ENEMY_VERMIN
                && enemies[i_count].type <= ENEMY_SERPENT)
                || enemies[i_count].type == ENEMY_SPECTRE)
                enemies[i_count].behaviour = BEHAVIOUR_GUARD;
        }

        /* the last two items are out in the open */
        else
            do {
                x = rand () % 16;
                y = rand () % 16;
            } while (map[x][y] != 0x10);

        /* place the item at the chosen map location */
        map[x][y] |= items_list[i_count];
    }
}

/**
 * Ascertain player direction from a keypress.
 * @param key is the value of the key pressed
 * @returns an integer corresponding to the direction keys.
 */
static int get_direction (int key)
{
    /* local variables */
    int direction = DIR_NONE;

    /* set the direction if exactly one direction key is pressed */
    if (key == KEY_UP || key == KEY_KEYPAD_UP)
        direction = DIR_UP;
    else if (key == KEY_DOWN || key == KEY_KEYPAD_DOWN)
        direction = DIR_DOWN;
    else if (key == KEY_LEFT || key == KEY_KEYPAD_LEFT)
        direction = DIR_LEFT;
    else if (key == KEY_RIGHT || key == KEY_KEYPAD_RIGHT)
        direction = DIR_RIGHT;

    /* return the direction, using DIR_NONE to signify none */
    return direction;
}

/**
 * Move the hero in a given direction.
 * @param direction is the direction to move in.
 * @returns 1 if the hero successfully moves.
 */
static int move_hero (int direction)
{
    /* local variables */
    int xd, yd; /* X and Y direction offsets */
    int dest; /* contents of destination square */

    /* determine X and Y direction offsets and see what's there */
    switch (direction) {
        case DIR_UP: xd = 0; yd = -1; break;
        case DIR_DOWN: xd = 0; yd = 1; break;
        case DIR_LEFT: xd = -1; yd = 0; break;
        case DIR_RIGHT: xd = 1; yd = 0; break;
    }
    dest = map[hero_x + xd][hero_y + yd];

    /* if it's a wall, we can't go there */
    if (dest < 4)
        return 0;

    /* if destination square is clear, move there */
    else if ((dest & 0xf0) == 0x10) {

        /* move the hero */
        map[hero_x + xd][hero_y + yd] ^= 0x30;
        map[hero_x][hero_y] ^= 0x30;
        update_map_tile (hero_x + xd, hero_y + yd,
            map[hero_x + xd][hero_y + yd]);
        update_map_tile (hero_x, hero_y, map[hero_x][hero_y]);
        hero_x += xd;
        hero_y += yd;
        display_map (hero_x, hero_y);

        /* update the message */
        if (map[hero_x][hero_y] & 0xf)
            display_message (item_message[map[hero_x][hero_y] & 0xf]);
        else
            blank_message ();
    }
    
    /* if the destination square is an enemy, attack it */
    else if (dest & 0x80)
        attack_enemy (hero_x + xd, hero_y + yd);

	/* if the destination square is the necromancer, attack him */
	else if (dest == 0x30)
		attack_necromancer ();

    /* we performed an action! */
    return 1;
}

/**
 * Bring up the main menu and perform the selected option.
 * @returns 1 if the level should be finished.
 */
static int main_menu (void)
{
    /* local variables */
    int option, /* option currently selected */
        finished, /* finished selecting option */
        key; /* key press */
    
    /* decide on the default option */
    if ((map[hero_x][hero_y] & 0xf) == 0)
        option = MENU_INVENTORY;
    else if ((map[hero_x][hero_y] & 0xf) == 0xf)
        option = MENU_DESCEND;
    else
        option = MENU_PICK_UP;

    /* main menu loop */
    finished = 0;
    while (! finished) {
    
        /* display the current option and await a keypress */
        display_message (menu_option_text[option]);
        key = get_keypress ();
        
        /* see if the user has finished */
        if (key == 32 || key == 27)
            finished = 1;
        
        /* cycle through the menu options */
        else if (key == KEY_UP || key == KEY_KEYPAD_UP
            || key == KEY_LEFT || key == KEY_KEYPAD_LEFT)
            --option;
        else if (key == KEY_DOWN || key == KEY_KEYPAD_DOWN
            || key == KEY_RIGHT || key == KEY_KEYPAD_RIGHT)
            ++option;
        
        /* choose a menu option based on a shortcut */
        else if (key == '?' || key == 'h') {
            option = MENU_HELP;
            finished = 1;
        } else if (key == '.') {
            option = MENU_WAIT;
            finished = 1;
        } else if (key == 'i') {
            option = MENU_INVENTORY;
            finished = 1;
        } else if (key == 't') {
            option = MENU_PICK_UP;
            finished = 1;
        } else if (key == 'e') {
            option = MENU_EAT;
            finished = 1;
        } else if (key == 'q') {
            option = MENU_QUAFF;
            finished = 1;
        } else if (key == 's' || key == '>') {
            option = MENU_DESCEND;
            finished = 1;
        } else if (key == 'x') {
            option = MENU_EXIT;
            finished = 1;
        }
        

        /* make sure we're pointing at a valid menu option */
        if (option < MENU_HELP)
            option = MENU_CANCEL;
        else if (option > MENU_CANCEL)
            option = MENU_HELP;
    }
    blank_message ();
    
    /* return the option chosen */
    return (key == 27) ? MENU_CANCEL : option;
}

/**
 * Pick up an item.
 * Returns 1 if hero managed to pick something up.
 */
static int pick_up_item (void)
{
    /* local variables */
    int item, /* what are we trying to pick up? */
        i; /* inventory slot counter */

    /* make sure there is something to pick up */
    item = map[hero_x][hero_y] & 0xf;
    if (item == 0 || item == 0xf) {
        display_message ("NOTHING TO TAKE!");
        return 0;
    }
    
    /* if this is gold, just add it to the total */
    if (item <= 2) {
        gold += (item == 1) ? 20 : 50;
        display_gold (gold);
        gold_noise ();
    }
    
    /* otherwise put it in the inventory */
    else {

        /* find a free inventory slot */
        for (i = 0; i < 12; ++i)
            if (inventory[i] == 0)
                inventory_slot = i;
        if (inventory[inventory_slot]) {
            display_message ("YOUR BAG IS FULL");
            return 0;
        }
        
        /* put the item there */
        inventory[inventory_slot] = item;
        display_inventory (inventory_slot, item);
        if (inventory_slot < 2)
            recalc_modifiers ();
        inventory_noise ();
    }
    
    /* remove item from map and show message */
    map[hero_x][hero_y] &= 0xf0;
    display_message (" PICKED UP ITEM ");
    return 1;
}

/**
 * Find an apple in the inventory and eat it.
 * @returns 1 if hero managed to eat.
 */
static int eat_something (void)
{
    /* local variables */
    int i, /* item counter */
        success; /* 1 if item eaten */

    /* search for an apple */
    for (i = 11; i >= 0; --i)
        if (inventory[i] == ITEM_APPLE)
            inventory_slot = i;

    /* did we find an apple? */
    if (inventory[inventory_slot] == ITEM_APPLE) {
        success = consume_item ();
        if (success)
            inventory_noise ();
        return success;
    }

    /* complain that we didn't find an apple */
    display_message (" NOTHING TO EAT ");
    return 0;
}

/**
 * Find a potion in the inventory and drink it.
 * @returns 1 if the hero managed to drink.
 */
static int quaff_something (void)
{
    /* local variables */
    int i, /* item counter */
        success; /* 1 if potion quaffed */
    
    /* search for a ption */
    for (i = 11; i >= 0; --i)
        if (inventory[i] == ITEM_POTION)
            inventory_slot = i;

    /* did we find a potion? */
    if (inventory[inventory_slot] == ITEM_POTION) {
        success = consume_item ();
        if (success)
            inventory_noise ();
        return success;
    }
    
    /* complain that we didn't find a potion */
    display_message ("NOTHING TO QUAFF");
    return 0;
}

/**
 * Descend the stairs.
 * @returns 1 if successful.
 */
static int descend_stairs (void)
{
    /* local variables */
    int success = 0; /* 1 when hero descends */
    int difficulty; /* difficulty to increase */

    /* are we at the stairs? */
    if (map[hero_x][hero_y] == 0x2f) {

        /* increase level and mark this level done */
        ++level;
        level_over = 1;

        /* increase some difficulty */
        if (level < 24)
            do {
                difficulty = 1 + rand () % 3;
                if (difficulty == 1 && enemy_min < enemy_max)
                    ++enemy_min;
                else if (difficulty == 2 && enemy_max < ENEMY_DAEMON)
                    ++enemy_max;
                else if (difficulty == 3 && enemy_anger < 8)
                    ++enemy_anger;
                else
                    difficulty = 0;
            } while (! difficulty);

        /* set message and mark success */
        display_message ("DESCENDING STEPS");
        update_map_tile (hero_x, hero_y, 0x1f);
        display_map (hero_x, hero_y);
        stairs_noise ();
        success = 1;
    }

    /* we're not at the stairs, give an error message */
    else
        display_message (" NO STAIRS HERE ");

    /* return a success code */
    return success;
}

/**
 * Perform the "wait a minute" action.
 * @returns 1 - this action never failes.
 */
static int wait_action (void)
{
    display_message (" TIME PASSES... ");
    return 1;
}

/**
 * Do some inventory action.
 * @returns 1 if an action was successfully performed.
 */
static int inventory_action (void)
{
    /* local variables */
    int key, /* key pressed */
        option, /* option returned */
        action; /* 1 if an action was successfully performed */

    /* main inventory loop */
    action = 0;
    option = MENU_CANCEL;
    while (option == MENU_CANCEL) {
    
        /* highlight inventory and get a keypress */
        highlight_inventory (inventory_slot);
        if (inventory[inventory_slot])
            display_message (item_message[inventory[inventory_slot]]);
        else
            blank_message ();
        key = get_keypress ();
        display_inventory (inventory_slot, inventory[inventory_slot]);
        if (inventory[inventory_slot])
            blank_message ();

        /* move around the inventory */
        if ((key == KEY_UP || key == KEY_KEYPAD_UP)
            && inventory_slot > 1)
            inventory_slot -= 2;
        else if ((key == KEY_DOWN || key == KEY_KEYPAD_DOWN)
            && inventory_slot < 10)
            inventory_slot += 2;
        else if ((key == KEY_LEFT || key == KEY_KEYPAD_LEFT)
            && (inventory_slot % 2))
            --inventory_slot;
        else if ((key == KEY_RIGHT || key == KEY_KEYPAD_RIGHT)
            && ! (inventory_slot % 2))
            ++inventory_slot;

        /* process menu key and shortcuts */
        else if (key == 32 || key == 13)
            option = inventory_menu ();
        else if (key == '?' || key == 'h')
            option = MENU_INV_HELP;
        else if (key == 'w')
            option = MENU_INV_WIELD;
        else if (key == 'p')
            option = MENU_INV_PUT_AWAY;
        else if (key == 'c')
            option = MENU_INV_CONSUME;
        else if (key == 'e' && inventory[inventory_slot] == ITEM_APPLE)
            option = MENU_INV_CONSUME;
        else if (key == 'q' && inventory[inventory_slot] == ITEM_POTION)
            option = MENU_INV_CONSUME;
        else if (key == 'd')
            option = MENU_INV_DISCARD;
        else if (key == 27)
            option = MENU_INV_CLOSE;
    }

    /* process the option chosen */
    switch (option) {
        case MENU_INV_CLOSE: break;
        case MENU_INV_HELP: show_keys (); break;
        case MENU_INV_WIELD: action = wield_item (); break;
        case MENU_INV_PUT_AWAY: action = put_item_away (); break;
        case MENU_INV_CONSUME: action = consume_item (); break;
        case MENU_INV_DISCARD: action = discard_item (); break;
    }

    /* return true if an action was performed */
    if (action)
        inventory_noise ();
    return action;
}

/**
 * Confirm that the player wishes to restart the game.
 */
static void confirm_new_game (void)
{
    int key; /* key press Y or N */
    display_message ("START NEW GAME? ");
    do {
        key = get_keypress ();
    } while (key != 'y' && key != 'n');
    blank_message ();
    game_over = (key == 'y');
}

/**
 * Confirm that the player wishes to exit the game.
 */
static void confirm_exit (void)
{
    int key; /* key press Y or N */
    display_message ("REALLY EXIT? Y/N");
    do {
        key = get_keypress ();
    } while (key != 'y' && key != 'n');
    blank_message ();
    game_quit = (key == 'y');
}

/**
 * Work out and perform an action for an individual enemy.
 * @param enemy_no is the enemy number.
 */
static void individual_enemy_action (int enemy_no)
{
    /* local variables */
    int distance; /* distance between enemy and hero */

    /* work out if we should be chasing */
    distance = abs (enemies[enemy_no].x - hero_x)
        + abs (enemies[enemy_no].y - hero_y);

    /* chase the player if close enough */
    if (distance <= enemies[enemy_no].aggression)
        enemy_pathfind (enemy_no, hero_x, hero_y, 1);

    /* if wandering, continue wandering */
    else if (enemies[enemy_no].behaviour == BEHAVIOUR_WANDER)
        enemy_pathfind (enemy_no, enemies[enemy_no].dest_x,
            enemies[enemy_no].dest_y, 0);
}

/*----------------------------------------------------------------------
 * Public Level Functions.
 */

/**
 * Initialise some game variables.
 */
void init_game (void)
{
    level = 1;
    gold = 0;
    enemy_min = ENEMY_BAT;
    enemy_max = ENEMY_BAT;
    enemy_anger = 0;
    experience = 0;
    necro_health = 24;
    game_over = 0;
    game_quit = 0;
}

/**
 * Generate the hero.
 */
void generate_hero (void)
{
    /* local variables */
    int stat_increases = 15;
    int *stat_ptr;

    /* start with minimum stats */
    hero_strength = 2;
    hero_agility = 2;
    hero_intelligence = 2;
    
    /* increase stats to a sensible level */
    while (stat_increases) {
        switch (rand () % 3) {
            case 0: stat_ptr = &hero_strength; break;
            case 1: stat_ptr = &hero_agility; break;
            case 2: stat_ptr = &hero_intelligence; break;
        }
        if (*stat_ptr == 12) continue;
        ++*stat_ptr;
        --stat_increases;
    }
    
    /* set the modifiers */
    strength_modifier = 0;
    agility_modifier = 0;
    intelligence_modifier = 0;
    
    /* set health to 21 */
    hero_health = 21;
}

/**
 * Initialise the inventory.
 */
void init_inventory (void)
{
    /* local variables */
    static int start_items[12] = {
        0, 0,
        0, 0, 0, 0, 0, 0, 0, 4, 3, 3
    };
    int count;
    
    /* copy the starting items into the inventory */
    for (count = 0; count < 12; ++count)
        inventory[count] = start_items[count];
    inventory_slot = 0;
}

/**
 * Show the player stats.
 */
void show_stats (void)
{
    display_stat (0, hero_strength, strength_modifier);
    display_stat (1, hero_agility, agility_modifier);
    display_stat (2, hero_intelligence, intelligence_modifier);
    display_health (hero_health,
        hero_strength + hero_agility + hero_intelligence);
}

/**
 * Show the gold counter.
 */
void show_gold (void)
{
    display_gold (gold);
}

/**
 * Show the inventory.
 */
void show_inventory (void)
{
    int i;
    for (i = 0; i < 12; ++i)
        display_inventory (i, inventory[i]);
}

/**
 * Ask the player for their name.
 */
void ask_name (void)
{
    /* local variables */
    char prompt[17];
    int ch;
    
    /* editing loop */
    do {

        /* prompt */
        sprintf (prompt, "NAME? %-10s", name);
        display_message (prompt);

        /* get a key. Discard special keys */
        ch = toupper (get_keypress ());

        /* if it's a printable character, add it to the name */
        if (ch >= ' ' && ch <= '_' && ch != ':' && strlen (name) < 10) {
            name[strlen (name) + 1] = '\0';
            name[strlen (name)] = ch;
        }

        /* if it's backspace, delete the last character */
        else if (ch == 8 && *name != '\0')
            name[strlen (name) - 1] = '\0';
        
    } while (ch != 13);

    /* greet the player */
    display_message (" PLEASE WAIT... ");
}

/**
 * Initialise the map for a level
 */
void generate_map (void)
{
    /* local variables */
    static int cell_order[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    int cell_map[9];

    /* build up the 3x3 cell network */
    scramble_cells (cell_order);
    generate_paths (cell_order, cell_map);
    generate_chambers (cell_map);

    /* turn the cell network into a 16x16 map */
    fill_map ();
    open_cells (cell_map);
    decorate_walls ();
}

/**
 * Place the hero on the current level
 */
void place_hero (void)
{
    do {
        hero_x = rand () % 16;
        hero_y = rand () % 16;
    } while (map[hero_x][hero_y] != 0x10);
    map[hero_x][hero_y] = 0x20;
    level_over = 0;
}

/**
 * Place the level's objective (staircase or necromancer)
 */
void place_objective (void)
{
    /* local variables */
    int x, y; /* location choice */

    /* choose a suitable location */
    do {
        x = rand () % 16;
        y = rand () % 16;
    } while (map[x][y] != 0x10
        || abs (x - hero_x) + abs (y - hero_y) < 10);

    /* if we're on level 24, place the necromancer */
    if (level == 24) {
        necro_x = x;
        necro_y = y;
        map[x][y] = 0x30;
    }

    /* otherwise place the stairs down */
    else
        map[x][y] = 0x1f;
}

/**
 * Place the level's eight enemies
 */
void place_enemies (void)
{
    int e_count; /* enemy count */
    int x, y; /* temporary variables to look at map */

    /* loop through eight enemies */
    for (e_count = 0; e_count < 8; ++e_count) {

        /* find a suitable place for this enemy */
        do {
            x = rand () % 16;
            y = rand () % 16;
        } while (map[x][y] != 0x10
            || abs (x - hero_x) + abs (y - hero_y) < 7);

        /* set the enemy details */
        enemies[e_count].x = x;
        enemies[e_count].y = y;
        enemies[e_count].type = enemy_min
            + rand () % (enemy_max - enemy_min + 1);
        enemies[e_count].health
            = enemy_stats[enemies[e_count].type].strength
            + enemy_stats[enemies[e_count].type].agility;
        enemies[e_count].behaviour = rand () % 2;
        if (enemies[e_count].behaviour == BEHAVIOUR_WANDER) {
            enemies[e_count].dest_x = rand () % 16;
            enemies[e_count].dest_y = rand () % 16;
        }
        enemies[e_count].aggression = enemy_anger + rand () % 3;

        /* put the enemy on the map */
        map[x][y] = 0x80 | ((enemies[e_count].type) * 0x10);
    }
}

/**
 * Place the level's items
 */
void place_items (void)
{
    int quick_inventory, /* a bit-encoded list of what the player has */
        items_list[6]; /* a list of items to place on this level */
    quick_inventory = get_quick_inventory ();
    build_items_list (items_list, quick_inventory);
    place_listed_items (items_list);
}

/**
 * Generate the drawn map from a new level.
 */
void generate_drawn_map (void)
{
    int x, y;
    for (x = 0; x < 16; ++x)
        for (y = 0; y < 16; ++y)
            update_map_tile (x, y, map[x][y]);
}

/**
 * Show the level number
 */
void show_level (void)
{
    char level_text[9];
    sprintf (level_text, "LEVEL %02d", level);
    display_heading (level_text);
}

/**
 * Show the map window
 */
void show_map (void)
{
    display_map (hero_x, hero_y);
}

/**
 * Get and process player action
 * @returns 1 if an action was successfully performed.
 */
int player_action (void)
{
    /* local variables */
    int direction,
        key,
        option,
        action = 0; /* changed to 1 if hero does something */

    /* continue this loop till the player does something */
    do {

        /* see what the player wants to do */
        option = MENU_CANCEL;
        key = get_keypress ();

        /* if this is a movement key, then move */
        direction = get_direction (key);
        if (direction != DIR_NONE)
            action = move_hero (direction);
        
        /* if this is a menu key, call up the menu */
        else if (key == 13 || key == 32)
            option = main_menu ();
        
        /* respond to the various shortcut keys */
        else if (key == '?' || key == 'h')
            option = MENU_HELP;
        else if (key == '.')
            option = MENU_WAIT;
        else if (key == 'i')
            option = MENU_INVENTORY;
        else if (key == 't')
            option = MENU_PICK_UP;
        else if (key == 'e')
            option = MENU_EAT;
        else if (key == 'q')
            option = MENU_QUAFF;
        else if (key == 's' || key == '>')
            option = MENU_DESCEND;
        else if (key == 'n')
            option = MENU_NEW_GAME;
        else if (key == 'x' || key == 27)
            option = MENU_EXIT;

        /* process a menu option */
        switch (option) {
            case MENU_CANCEL: break;
            case MENU_HELP: show_keys (); break;
            case MENU_WAIT: action = wait_action (); break;
            case MENU_INVENTORY: action = inventory_action (); break;
            case MENU_PICK_UP: action = pick_up_item (); break;
            case MENU_EAT: action = eat_something (); break;
            case MENU_QUAFF: action = quaff_something (); break;
            case MENU_DESCEND: action = descend_stairs (); break;
            case MENU_NEW_GAME: confirm_new_game (); break;
            case MENU_EXIT: confirm_exit (); break;
        }
    } while (! (action || level_over || game_over || game_quit));

    /* return 1 if the screen needs updating after this action */
    return action;
}

/**
 * Generate and process enemy action
 */
void enemy_action (void)
{
    /* local variables */
    int e; /* enemy counter */
    
    /* loop through all the enemies */
    for (e = 0; e < 8; ++e)
        if (enemies[e].health > 0)
            individual_enemy_action (e);
}

/**
 * Generate and process the necromancer's action.
 */
void necro_action (void)
{
    /* local variables */
    int attack, /* enemy's attack value */
        defence, /* hero's defence value */
        damage, /* damage caused */
        update; /* 1 if the screen needs updating */

	/* if the player is not adjacent, do nothing */
	if (level < 24)
		return;
	if (abs (hero_x - necro_x) + abs (hero_y - necro_y) > 1)
		return;

    /* determine attack and defence */
    attack = 20;
    if (inventory[0] == ITEM_AMULET || inventory[1] == ITEM_AMULET)
        defence = hero_intelligence + intelligence_modifier;
    else
        defence = hero_agility + agility_modifier;
    
    /* conduct the attack */
    damage = (1 + attack + dice (2)) - (defence + dice (2));
    if (damage < 0)
        damage = 0;
    
    /* register the damage */
    hero_health -= damage;
    if (hero_health < 0)
        hero_health = 0;
    update = 0;

    /* process a miss */
    if (! damage)
        display_message ("SORCEROR MISSES ");

    /* process a non-fatal hit */
    else if (hero_health) {
        display_message ("SORCEROR HIT YOU");
        display_health (hero_health, hero_strength + hero_agility
            + hero_intelligence);
        hit_noise ();
    }

    /* process a fatal hit */
    else {
        display_message ("SORCEROR HIT YOU");
        display_health (hero_health, hero_strength + hero_agility
            + hero_intelligence);
        map[hero_x][hero_y] = (map[hero_x][hero_y] & 0xf) | 0x10;
        update_map_tile (hero_x, hero_y, map[hero_x][hero_y]);
        game_over = 1;
    }
}

/**
 * See if the level is over.
 * @returns 1 if the level is over, 0 if not.
 */
int level_is_over (void)
{
    return level_over
        || game_quit
        || game_over
        || (hero_health == 0)
        || (necro_health == 0);
}

/**
 * See if the game is over.
 * @returns 1 if the game is over, 0 if not.
 */
int game_is_over (void)
{
    return game_quit
        || game_over
        || (hero_health == 0)
        || (necro_health == 0);
}

/**
 * Return a code to say who won the game, if anyone.
 * @returns +1 if the hero won, -1 if the hero lost, 0 otherwise.
 */
int who_won (void)
{
    return
        (necro_health == 0)
        - (hero_health == 0);
}

/**
 * See if user wants to quit.
 * @returns 1 if the player wants to quit, 0 if not.
 */
int player_quit (void)
{
    return game_quit;
}

/**
 * Return the player name.
 * @returns pointer to player name.
 */
char *get_name (void)
{
    return name;
}

/**
 * Return the player's gold
 * @returns the gold
 */
int get_gold (void)
{
    return gold;
}

/**
 * Return the level number.
 * @returns the current level number.
 */
int get_level (void)
{
    return level;
}
