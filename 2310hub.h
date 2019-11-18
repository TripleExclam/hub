#ifndef _2310HUB_H_
#define _2310HUB_H_

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h> 
#include "utilities.h"

#define ERROR_INCORRECT_ARGS 1
#define ERROR_INVALID_THRESHOLD 2 
#define ERROR_DECK 3
#define ERROR_CARD_COUNT 4
#define ERROR_PLAYER 5
#define ERROR_PLAYER_EOF 6
#define ERROR_PLAYER_MESSAGE 7
#define ERROR_CARD_CHOICE 8
#define ERROR_SIGHUP 9

#define EXPECTED_HUB_ARGS 4
#define NON_PLAYER_ARGS 3
#define FAIL '%'

/**
 * Representation of a player.
 * 
 * @param hand - An array of cards
 * @param handSize - The number of cards
 * @param score - rounds won by the player
 * @param specialCards - D cards won by the player
 * @param track - The players process ID
 * @param read - A file to read the players messages
 * @param write - A file to write the player messages
 */ 
typedef struct {
    Card* hand;
    int handSize;
    int score;
    int specialCards;
    pid_t track;
    FILE* read;
    FILE* write;
} Player;

/**
 * Stores all information pertaining to the game
 *
 * @param threshold - The number of D cards needed for an additional score
 * @param playerCount - The number of players
 * @param deckSize - The number of cards stored in the hub.
 * @param round - The number of rounds to play
 * @param deck - All cards in the game
 * @param players - All the players in the game
 */ 
typedef struct {
    int threshold;
    int playerCount;
    int deckSize;
    int round;
    Card* deck;
    Player* players;
} HubInfo;

/* Game Running functions */
void exit_game(int exitCondition);
void init_players(HubInfo* game, char** argv);
void run_game(HubInfo* game);
void handle_death(int sig);
void end_players(HubInfo* game);

/* File IO functions */
void parse_deck(HubInfo* game, char* deck);
bool send_cards(Player* player);
void message_players(Player** players, char* message);
void send_played(HubInfo* game, int player, Card played);
void send_new_round(HubInfo* game, int leadPlayer);  
void output_cards(Card* played, int cardCount);
bool create_player(Player* newProcess, char** args);

/* Helper functions */
int play_round(HubInfo* game, int leadPlayer);
Card parse_play(HubInfo* game, char* line, int currentPlayer);
void deal_cards(HubInfo* game);

#endif //_2310HUB_H_
