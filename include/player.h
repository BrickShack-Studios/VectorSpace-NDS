#ifndef PLAYER_H
#define PLAYER_H

#include <nds.h>

#include "meteor.h"

typedef struct Player
{
    int32* position;
    int32* velocity;
    
    uint8 width;
    uint8 height;

    int32 speed;
} Player;

Player* newPlayer();

/**
   Updates the player's position using their velocity as-is. If they moved, it also moves the sprite.
 */
void movePlayer(Player* player);

void playerControl(Player* player);

void autoPilot(Player* player, Meteor** meteors, uint8 mCount);

#endif
