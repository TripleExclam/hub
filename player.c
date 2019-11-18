#include "player.h"

/**
 * Set up the players
 * 
 * @param argc - An array of arguments.
 * @param argv - The number of arguments.
 * @param playCard - A function to play a card.
 */ 
void init_game(Card (*playCard)(struct PlayerInfo*, bool, Card, bool), 
        int argv, char** argc) {
    if (argv != 5) {
        exit_game(ERROR_INCORRECT_ARGS);
    } 
    PlayerInfo game;
    game.playCard = playCard;

    affirm_input(&game, argc);

    read_hand(&game);
    
    run_round(&game);

    free(game.hand);
    exit(NORMAL_EXIT);
}

/**
 * Run the game round to round
 * 
 * @param game - information about that game.
 */ 
void run_round(PlayerInfo* game) {
    char* line;
    int leadPlayer = 0;
    int wonOnD = 0;
    while (read_new_line(stdin, &line)) {
        if (game->handSize == 0 || check_command(line, RECIEVE_NEWROUND, false)
                || (leadPlayer = read_int(strtok(line, RECIEVE_NEWROUND))) < 0 
                || leadPlayer >= game->playerCount) {
            exit_game(ERROR_INVALID_MESSAGE);
        }
        free(line);
        watch_round(game, leadPlayer, &wonOnD);
    }
}

/**
 * Handle a round of the game.
 * 
 * @param game - Information about the game state.
 * @param leadPlayer - Player going first.
 * @param wonOnD - Number of D cards won.
 */ 
void watch_round(PlayerInfo* game, int leadPlayer, int* wonOnD) {
    int currentPlayer = leadPlayer;
    char* line;
    int winner = leadPlayer;
    int cardCount = 0;
    Card* playedCard = malloc(sizeof(Card) * game->playerCount);
    int seenD = 0;

    // Read and interpret the data recieved during the game.
    do {
        // Check who's turn it is and either play a card or read it.
        if (currentPlayer == game->playerNum) {
            playedCard[cardCount] = game->playCard(game, 
                    (leadPlayer == currentPlayer), playedCard[0], 
                    (seenD && *wonOnD >= game->threshold - 2));
        } else {
            read_new_line(stdin, &line);
            playedCard[cardCount] = parse_play(line, currentPlayer);
            free(line);
        }

        // Update the winner
        if (playedCard[cardCount].suit == playedCard[0].suit 
                && playedCard[cardCount].rank > playedCard[0].rank) {
            winner = currentPlayer;
        }
        
        // Track all D cards played.
        seenD += (playedCard[cardCount++].suit == SPECIAL_SUIT);
        currentPlayer = (currentPlayer + 1) % game->playerCount;
    } while(currentPlayer != leadPlayer);

    if (winner == game->playerNum) {
        game->score++;
        game->specialCards += seenD;
    }
    
    fprintf(stderr, "Lead player=%d:", leadPlayer);
    for (int i = 0; i < cardCount; i++) {
        (*wonOnD) += (playedCard[i].suit == SPECIAL_SUIT);
        fprintf(stderr, " %c.%c", playedCard[i].suit, playedCard[i].rank);
    }
    fprintf(stderr, "\n");

    free(playedCard);
}

/**
 * Read a card from the hub.
 * 
 * @param line - A string.
 * @param expectedPlayer - the player whose turn it is 
 */ 
Card parse_play(char* line, int expectedPlayer) { 
    char* newCard;

    // Check the command, player and card recieved from the hub is correct.
    if (check_command(line, RECIEVE_PLAYED, false) || read_int(strtok(
            line + strlen(RECIEVE_PLAYED), ",")) != expectedPlayer 
            || (newCard = strtok(NULL, ",")) == NULL || !check_card(newCard)) {
        exit_game(ERROR_INVALID_MESSAGE);
    } 
    return (Card) {.suit = newCard[0], .rank = newCard[1]};
}

/**
 * Read in a hand.
 * 
 * @param game - Information about the game state
 */ 
void read_hand(PlayerInfo* game) {
    char* line;
    read_new_line(stdin, &line);

    if (check_command(line, RECIEVE_HAND, false)) {
        exit_game(ERROR_INVALID_MESSAGE);
    }
    parse_hand(line, &game->hand, game->handSize);
    free(line);
}

/**
 * Read a hand from the player.
 * 
 * @param line - A string of text.
 * @param hand - An array of Cards.
 * @param handSize - The size of the hand.
 */ 
void parse_hand(char* line, Card** hand, int handSize) {
    int tempCount = 0;
    int cardCount = read_int(strtok(line + strlen(RECIEVE_HAND), ","));

    if (cardCount < 1 || cardCount != handSize) {
        exit_game(ERROR_INVALID_MESSAGE);
    }

    (*hand) = malloc(sizeof(Card) * cardCount);

    char* currentCard;
    // Split the input by comma and fill the players hand.
    while ((currentCard = strtok(NULL, ",")) != NULL) {
        // Prevent oversize.
        if (tempCount == cardCount 
                || !check_card(currentCard)) {
            exit_game(ERROR_INVALID_MESSAGE);
        }

        (*hand)[tempCount++] = (Card) {.suit = currentCard[0], 
                .rank = currentCard[1]};
    }
    // prevent undersize.
    if (tempCount < cardCount - 1) {
        exit_game(ERROR_INVALID_MESSAGE);
    }
}

/**
 * Check the command line arguments of the function.
 * 
 * @param game - Information about the game state.
 * @param argc - The command line arguments.
 */ 
void affirm_input(PlayerInfo* game, char** argc) {
    // Since read int returns -1 for NAN, these cases also check for it.
    if ((game->playerCount = read_int(argc[1])) < 2) {
        exit_game(ERROR_BAD_PLAYERS);

    } else if ((game->playerNum = read_int(argc[2])) >= game->playerCount 
            || game->playerNum < 0) {
        exit_game(ERROR_INVALID_POS);

    } else if ((game->threshold = read_int(argc[3])) < 2) {
        exit_game(ERROR_BAD_THRESHOLD);

    } else if ((game->handSize = read_int(argc[4])) < 1) { 
        exit_game(ERROR_BAD_HSIZE);

    }
    printf("@"); // Successfully, read the cArgs.
    fflush(stdout);
}

/* Exits the game with specifid error Code
 *
 * @param exitCode - what to exit with
 */
int exit_game(int exitCondition) {
    const char* messages[] = {"",
            "Usage: player players myid threshold handsize\n",
            "Invalid players\n",
            "Invalid position\n",
            "Invalid threshold\n",
            "Invalid hand size\n",
            "Invalid message\n",
            "EOF\n"};   
    fputs(messages[exitCondition], stderr);
    exit(exitCondition);
}

/**
 * Intermediary between read_line and the player.
 * 
 * @param line - A string of text.
 * @param toRead - The file to read from.
 */ 
char* read_new_line(FILE* toRead, char** line) {
    char* lineCheck = read_line(toRead, line);

    // Handle events that can happen anytime.
    if (lineCheck == NULL) {
        exit_game(ERROR_UNEXPECTED_EOF);
    } else if (!strcmp(lineCheck, RECIEVE_GAMEOVER)) {
        exit_game(NORMAL_EXIT);
    }
    return lineCheck;
}

/**
 * Find the largest card in a hand given an order.
 * 
 * @param hand - An array of cards.
 * @param handSize - The number of cards in the hand.
 * @param compRank - A function to compare two cards.
 * @param order - A specific order of cards to follow.
 */ 
Card find_extremum(Card* hand, int handSize, 
        int (*compRank)(int, int), char* order) {
    Card extremum = (Card) {.suit = DORMANT_CHAR, .rank = DORMANT_CHAR};
    for (int i = 0; i < SUIT_COUNT; i++) {
        for (int j = 0; j < handSize; j++) {
            if (order[i] == hand[j].suit) {
                if (extremum.suit == DORMANT_CHAR) {
                    extremum = hand[j];
                } else {
                    extremum = (compRank(hand[j].rank, extremum.rank)) 
                            ? hand[j] : extremum;
                }
            }
        }
        // If a card is found, exit the loop.
        if (extremum.suit != DORMANT_CHAR) {
            break;
        }
    }
    return extremum;
}

