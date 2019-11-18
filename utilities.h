#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define NORMAL_EXIT 0

#define PLAYER_READY '@'
#define SPECIAL_SUIT 'D'

#define RECIEVE_HAND "HAND"
#define RECIEVE_NEWROUND "NEWROUND"
#define RECIEVE_PLAYED "PLAYED"
#define RECIEVE_GAMEOVER "GAMEOVER"
#define SEND_PLAY "PLAY"

#define BASE 10
#define CHAR_BUFFER 80

#define EXPECTED_ARGS 4

#define READ_END 0
#define WRITE_END 1

/**
 * A representation of a card
 * 
 * @param suit - A letter suit
 * @param rank - A hexadecimal rank
 */ 
typedef struct {
    char suit;
    char rank;
} Card;

/* Utilities */
char* string_of(int num, char** line);
int read_int(char* line);
int check_command(char* line, char* toCheck, bool matchLength);
char* read_line(FILE* toRead, char** line);
bool check_card(char* card);
int find_max(int o1, int o2);
int find_min(int o1, int o2);
bool rotate_hand(Card* hand, int* handSize, Card played);

#endif // _UTILITIES_H_