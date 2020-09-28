#ifndef GAMEEVENT_H
#define GAMEEVENT_H

// Represents the set of events that can occur during rubber contract bridge gameplay.
enum GameEvent{
    INITIALIZE, // Game state has been initialized and the game is ready to start
    BID_START, // Start of bidding phase of the game
    BID_RESTART, // Four pass calls were made with no bid and bidding starts again
    PLAYER_BID, // A player has completed their turn during the bidding phase
    BID_END, // The bidding phase of the game has been completed
    PLAY_START, // Start of the card-playing phase of the game
    PLAYER_MOVED, // A player has completed their turn during the card-playing phase
    TRICK_END, // A trick has been completed during the card-playing phase
    PLAY_END, // The card-playing phase of the game has been completed
    GAME_END, // The game has been completed
    MATCH_END // The match has been completed
};

#endif // GAMEEVENT_H