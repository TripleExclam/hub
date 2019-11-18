#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "utilities.h"

#define NORMAL_EXIT 0
#define ERROR_INCORRECT_ARGS 1
#define ERROR_BAD_PLAYERS 2 
#define ERROR_INVALID_POS 3
#define ERROR_BAD_THRESHOLD 4
#define ERROR_BAD_HSIZE 5
#define ERROR_INVALID_MESSAGE 6
#define ERROR_UNEXPECTED_EOF 7 

#define DORMANT_CHAR '!'

#define SUIT_COUNT 4

/**
 * Representation of a player.
 * 
 * @param hand - An array of cards
 * @param handSize - The number of cards
 * @param score - rounds won by the player
 * @param specialCards - D cards won by the player
 * @param playerCount - The number of players
 * @param threshold - The number of D cards needed for an additional score
 * @param playCard - A function to select a card from the players hand
 */ 
typedef struct PlayerInfo {
    int score;
    int specialCards;
    int playerCount;
    int playerNum;
    int threshold;
    int handSize;
    Card* hand;
    Card (*playCard)(struct PlayerInfo*, bool, Card, bool);
} PlayerInfo;


/* IO functions */
// Command Line Parsing
void affirm_input(PlayerInfo* game, char** argc);
// Card Reading
void read_hand(PlayerInfo* game);
void parse_hand(char* line, Card** hand, int handSize);
// Play reading
Card parse_play(char* line, int expectedPlayer);

/* Game Operation */
int exit_game(int exitCondition);
void init_game(Card (*playCard)(struct PlayerInfo*, bool, Card, bool), 
        int argv, char** argc);
void run_round(PlayerInfo* game);
void watch_round(PlayerInfo* game, int leadPlayer, int* winners); 
void make_move(PlayerInfo* game); 

/* Utility */
char* read_new_line(FILE* toRead, char** line);
Card find_extremum(Card* hand, int handSize, 
        int (*compRank)(int, int), char* order);

#endif // _PLAYER_H_
