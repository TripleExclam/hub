#include "2310hub.h"

/* Reference to all player process ID's. */
pid_t* processes;
// Necessary to kill processes in the signal handler.

int main(int argc, char** argv) {
    if (argc <= EXPECTED_HUB_ARGS) {
        exit_game(ERROR_INCORRECT_ARGS);
    }

    HubInfo game;

    if ((game.threshold = read_int(argv[2])) < 2) {
        exit_game(ERROR_INVALID_THRESHOLD);
    }

    game.playerCount = argc - NON_PLAYER_ARGS;

    // GLOBAL VARAIBLES.
    processes = malloc(sizeof(pid_t) * (game.playerCount + 1));
    for (int i = 0; i <= game.playerCount; i++) {
        processes[i] = -1;
    }

    parse_deck(&game, argv[1]);

    init_players(&game, argv);

    run_game(&game);

    end_players(&game);

    exit_game(NORMAL_EXIT);
}

/**
 * Tick each round over and send output to the players / terminal.
 *
 * @param game - Information about the game state.
 */ 
void run_game(HubInfo* game) {
    // run through each round.
    int lead = 0;
    while (game->round-- > 0) {
        lead = play_round(game, lead);
    }

    // Output scores.
    for (int i = 0; i < game->playerCount; i++) {
        Player competitor = game->players[i];
        competitor.score = (competitor.specialCards < game->threshold)
                ? competitor.score - competitor.specialCards 
                : competitor.score + competitor.specialCards;
        // Control spacing of scores.
        (i == 0) ? printf("%d:%d", i, competitor.score) : 
                printf(" %d:%d", i, competitor.score);
    }
    printf("\n");
}

/**
 * Inform player processes that a new round has begun.
 * 
 * @param game - Information about the game state.
 * @param leadPlayer - Player going first.
 */ 
void send_new_round(HubInfo* game, int leadPlayer) {
    for (int i = 0; i < game->playerCount; i++) {
        fprintf(game->players[i].write, "%s%d\n", 
                RECIEVE_NEWROUND, leadPlayer);
        fflush(game->players[i].write);
    }
}

/**
 * Inform player processes of a played card.
 * 
 * @param game - Information about the game state.
 * @param player - The player who played the card.
 * @param played - The card that was played.
 */ 
void send_played(HubInfo* game, int player, Card played) {
    for (int i = 0; i < game->playerCount; i++) {
        if (i == player) {
            continue;
        }
        fprintf(game->players[i].write, "%s%d,%c%c\n", 
                RECIEVE_PLAYED, player, played.suit, played.rank);
        fflush(game->players[i].write);
    }
}

/**
 * Output an array of cards to stdout
 * 
 * @param played the array to rpint
 * @param cardCount the array size
 */ 
void output_cards(Card* played, int cardCount) {
    printf("Cards=");
    for (int i = 0; i < cardCount; i++) {
        (i == 0) ? printf("%c.%c", played[i].suit, played[i].rank) : 
                printf(" %c.%c", played[i].suit, played[i].rank);
    }
    printf("\n");
}

/**
 * Read the card a player has played.
 * 
 * @param game - Information about the game state.
 * @param line - A string of text.
 * @param currentPlayer - The player whose turn it was.
 */ 
Card parse_play(HubInfo* game, char* line, int currentPlayer) {
    if (check_command(line, SEND_PLAY, false) 
            || !check_card(line += strlen(SEND_PLAY))) {
        end_players(game);
        exit_game(ERROR_PLAYER_MESSAGE);
    }

    Card toPlay = (Card) {.suit = line[0], .rank = line[1]};

    // Remove the card from the players hand.
    if (!rotate_hand(game->players[currentPlayer].hand, 
            &game->players[currentPlayer].handSize, toPlay)) {
        end_players(game);
        exit_game(ERROR_CARD_CHOICE);
    }
    return toPlay;
}

/**
 * Run a round of the game
 * 
 * @param game - Information about the game state.
 * @param leadPlayer - The player going first.
 */ 
int play_round(HubInfo* game, int leadPlayer) {
    char* line;
    int winner = leadPlayer;
    int specials = 0;
    int cardCount = 0;
    Card lead;
    Card played[game->playerCount];

    send_new_round(game, leadPlayer);
    printf("Lead player=%d\n", leadPlayer);
    
    // Main round loop
    while (cardCount < game->playerCount) {
        if (!read_line(game->players[leadPlayer].read, &line)) {
            end_players(game);
            exit_game(ERROR_PLAYER_EOF);
        } 
        played[cardCount] = parse_play(game, line, leadPlayer);
        
        // The first player is the lead.
        if (cardCount == 0) {
            lead = played[cardCount];
        } else if (played[cardCount].suit == lead.suit 
                && played[cardCount].rank > lead.rank) {
            lead = played[cardCount];
            winner = leadPlayer;
        }

        send_played(game, leadPlayer, played[cardCount]);
        // Track all special cards played in a round.
        specials += (played[cardCount++].suit == SPECIAL_SUIT);
        leadPlayer = (leadPlayer + 1) % game->playerCount;
        free(line);
    }
    game->players[winner].specialCards += specials;
    game->players[winner].score += 1;

    output_cards(played, cardCount);

    return winner;
}

/**
 * Read the deck from a file
 * 
 * @param game - Information about the game state.
 */ 
void parse_deck(HubInfo* game, char* deck) {
    FILE* deckFile;
    deckFile = fopen(deck, "r");
    if (!deckFile) {
        exit_game(ERROR_DECK);
    }

    char* line;
    int lineN = 0;

    if (!read_line(deckFile, &line) 
            || (game->deckSize = read_int(line)) <= 0) {
        exit_game(ERROR_DECK);
    } 

    free(line);

    game->deck = malloc(sizeof(Card) * game->deckSize);

    while (read_line(deckFile, &line)) {
        // check cards are within array size.
        if (lineN >= game->deckSize || !check_card(line)) {
            exit_game(ERROR_DECK);
        }
        game->deck[lineN++] = (Card) {.suit = line[0], .rank = line[1]};
        free(line);
    }

    fclose(deckFile);
    // check deck is not under sized.
    if (lineN != game->deckSize) {
        exit_game(ERROR_DECK);
    }
}

/**
 * Signal handler for when a process ends with SIGHUP.
 * 
 * @param sig - The signal recieved.
 */
void handle_death(int sig) {
    // Ignore SIGPIPE.
    int i = 0;
    if (sig != SIGPIPE) {
        if (processes != NULL) {
            while (processes[i] != -1) {
                kill(processes[i++], SIGKILL);
            }
        }
        exit_game(ERROR_SIGHUP);   
    } 
}

/**
 * Set up the player processes.
 * 
 * @param game - Information about the game state.
 * @param argv - A list of command line arguments.
 */ 
void init_players(HubInfo* game, char** argv) {
    game->players = malloc(sizeof(Player) * game->playerCount);
    
    // Set up to handle SIGHUP and suppress SIGPIPE from players.
    struct sigaction sa = {.sa_handler = handle_death};
    sigaction(SIGHUP, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);

    deal_cards(game);
    for (int i = 0; i < game->playerCount; i++) {
        char* args[EXPECTED_ARGS + 2];

        // Create the array of arguments to pass each player.
        args[0] = argv[i + 3];
        string_of(game->playerCount, &args[1]);
        string_of(i, &args[2]); 
        string_of(game->threshold, &args[3]);
        string_of(game->players[i].handSize, &args[4]);
        args[5] = NULL;

        if (!create_player(&game->players[i], args) 
                || !send_cards(&game->players[i])) {
            exit_game(ERROR_PLAYER);
        }

        // Populate global variables.
        processes[i] = game->players[i].track;
    }
}

/**
 * Send players their hands
 * 
 * @param player - An array of players.
 */ 
bool send_cards(Player* player) {
    fprintf(player->write, "HAND%d", player->handSize);
    for (int j = 0; j < player->handSize; j++) {
        fprintf(player->write, ",%c%c", 
                player->hand[j].suit, player->hand[j].rank);
    }
    fprintf(player->write, "\n");
    fflush(player->write);
    // Check that the player is legitimate.
    return fgetc(player->read) == PLAYER_READY;
}

/**
 * Kill all player processes.
 * 
 * @param game - Information about the game state.
 */ 
void end_players(HubInfo* game) {
    for (int i = 0; i < game->playerCount; i++) {
        // fflush occurs when the hub exits (inevitable if we're here).
        fprintf(game->players[i++].write, "%s\n", RECIEVE_GAMEOVER);
    }
}

/**
 * Assign some cards to each player
 * 
 * @param game - Information about the game state.
 */ 
void deal_cards(HubInfo* game) {
    // The number of cards for each player to recieve. Also the # of rounds.
    game->round = game->deckSize / game->playerCount;
    
    // Check that there are enough cards for each player.
    if (game->round == 0) {
        exit_game(ERROR_CARD_COUNT);
    }

    int offset;
    for (int i = 0; i < game->playerCount; i++) {
        game->players[i].hand = malloc(sizeof(Card) * game->round);
        game->players[i].handSize = game->round;

        // Assign cards from deck based on player number.
        offset = game->round * i;
        for (int j = offset; j < offset + game->round; j++) {
            game->players[i].hand[j - offset] = 
                    (Card) {.suit = game->deck[j].suit, 
                    .rank = game->deck[j].rank};
        }
    }
}

/**
 * Initialise a player process
 * 
 * @param newProcess - The name of the process to create.
 * @param args - The command line arguments to pass.
 */ 
bool create_player(Player* newProcess, char** args) {
    // Initialise scores
    newProcess->score = 0;
    newProcess->specialCards = 0;

    int send[2];
    int recieve[2];
    int error[2];
    
    pipe(send);
    pipe(recieve);
    pipe(error);
     
    if (!(newProcess->track = fork())) {
        // Child process
        close(send[READ_END]);
        close(recieve[WRITE_END]);
        close(error[READ_END]);

        dup2(send[WRITE_END], STDOUT_FILENO);
        dup2(recieve[READ_END], STDIN_FILENO);
        dup2(error[WRITE_END], STDERR_FILENO);
            
        execvp(args[0], args);
        
        // Inform the hub that the process failed.
        fprintf(stdin, "%c", FAIL);

        // Exit closes all files.
        exit(ERROR_PLAYER);
    }
    // Close unwanted fd's.
    close(error[WRITE_END]);
    close(send[WRITE_END]);
    close(recieve[READ_END]);

    newProcess->read = fdopen(send[READ_END], "r");
    newProcess->write = fdopen(recieve[WRITE_END], "w");

    return newProcess->read && newProcess->write;
}

/* Exits the game with specifid error Code
 *
 * @param exitCode - what to exit with
 */
void exit_game(int exitCondition) {
    const char* messages[] = {"",
            "Usage: 2310hub deck threshold player0 {player1}\n",
            "Invalid threshold\n",
            "Deck error\n",
            "Not enough cards\n",
            "Player error\n",
            "Player EOF\n",
            "Invalid message\n",
            "Invalid card choice\n",
            "Ended due to signal\n"};
    fputs(messages[exitCondition], stderr);
    exit(exitCondition);
}
