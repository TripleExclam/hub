#include "2310bob.h"

/**
 * Find the Card to play
 * 
 * @param specialMove - Does the player have a special move
 * @param isLead - If the player is the lead
 * @param game - information about the game state.
 * @return Card - The card to be played.
 */ 
Card play_card(PlayerInfo* game, bool isLead, Card lead, bool specialMove) {
    Card toPlay;

    if (isLead) {
        char order[SUIT_COUNT] = {'D', 'H', 'S', 'C'};
        toPlay = find_extremum(game->hand, game->handSize, find_min, order);
    } else { 
        // Minimum and maximum searches subject to specialMoves status.
        char order[SUIT_COUNT] = {lead.suit, 'C', 'D', 'H'};
        toPlay = find_extremum(game->hand, game->handSize, 
                (specialMove) ? find_max : find_min, order);
        if (toPlay.suit != lead.suit) {
            order[0] = 'S';
            // Adjust the order.
            if (specialMove) {
                order[2] = 'H';
                order[3] = 'D';
            }
            toPlay = find_extremum(game->hand, game->handSize, 
                    (specialMove) ? find_min : find_max, order);
        }
    }

    rotate_hand(game->hand, &game->handSize, toPlay);

    // Send the card to the game
    printf("PLAY%c%c\n", toPlay.suit, toPlay.rank);
    if (fflush(stdout) == EOF) {
        exit_game(ERROR_UNEXPECTED_EOF);
    }

    return toPlay;
}

int main(int argv, char** argc) {
    init_game(play_card, argv, argc);
}
