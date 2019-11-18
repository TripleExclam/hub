#include "utilities.h"

/* Check if a command has been included in a string
 * 
 * @param line The string to search for the command in
 * @param toCheck The command to check for.
 * @param lengthMatch whether or not the string lengths must be the same
 */
int check_command(char* line, char* toCheck, bool lengthMatch) {
    if (lengthMatch) {
        // Trim down the line.
        line += strlen(line) - strlen(toCheck);
    }  
    return strncmp(line, toCheck, strlen(toCheck));
}

/* Convert an integer into a string.
 *
 * @param num - The number to stringify.
 * @param line - The line to store a string in.
 */
char* string_of(int num, char** line) {
    int length = 1;
    // Calculate the number of digits.
    while ((num / (BASE * length)) != 0) {
        length++;
    }
    (*line) = malloc(sizeof(char) * (length + 1));
    sprintf(*line, "%d", num);
    return (*line);
}

/* Convert some characters into an integer.
 * Returns -1 if this fails.
 *
 * @param line - The characters to turn into an integer.
 */
int read_int(char* line) {
    if (line == NULL) {
        return -1;
    }
    char* error;
    int num;
    num = strtol(line, &error, BASE);
    if (strlen(error) > 0) {
        // Any non-integer characters read
        return -1;
    }
    return num;
}

/* Read a line of text
 *
 * @param f The stream to read from
 * @param line A variable to save to
 * @return The line that is read
 */
char* read_line(FILE* toRead, char** line) {
    int c;
    int lineL = 0;
    int charCount = CHAR_BUFFER;
    *line = malloc(sizeof(char) * charCount);
    while ((c = fgetc(toRead)) != '\n') {
        // Handle EOF seperately to \n
        if (c == EOF) {
            free(*line);  
            return NULL;
        }
        (*line)[lineL++] = c;
        // Check if more memory is needed.
        if (lineL + 1 >= charCount) {
            charCount *= 2;
            *line = realloc(*line, sizeof(char) * charCount);
        }
    }
    (*line)[lineL] = '\0';
    return *line;
}

/* Check if a card is valid
 *
 * @param card A playing card to check
 */
bool check_card(char* card) {
    if (strlen(card) != 2 || (card[0] != 'D' && card[0] != 'H' 
            && card[0] != 'C' && card[0] != 'S') 
            || (card[1] < '1' || (card[1] > '9' 
            && card[1] < 'a') || card[1] > 'f')) {
        return false;
    }
    return true;
}

/**
 * Find a maximum of two ints.
 */ 
int find_max(int o1, int o2) {
    return o1 > o2;
}

/**
 * Find a minimum of two ints.
 */ 
int find_min(int o1, int o2) {
    return o1 < o2;
}

/**
 * Shorten a hand if a card has been played
 * 
 * @param hand - An array of cards.
 * @param handSize - The number of cards in the hand.
 * @param Card - The card in the array to reduce.
 * @return Whether or not the hand was rotated.
 */ 
bool rotate_hand(Card* hand, int* handSize, Card played) {
    // True if played matches a card in the hand
    bool rotate = false;
    for (int i = 0; i < *handSize; i++) {
        if (hand[i].suit == played.suit && hand[i].rank == played.rank) {
            rotate = true;
        }
        hand[i] = (rotate && i < *handSize - 1) ? hand[i + 1] : hand[i];
    }
    (*handSize)--;
    return rotate;
}
