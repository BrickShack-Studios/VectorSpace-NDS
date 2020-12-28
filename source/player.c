#include <nds.h>
#include <nf_lib.h>

#include "player.h"
#include "constants.h"
#include "vector.h"
#include "meteor.h"

Player* newPlayer()
{
    Player* player = malloc(sizeof(Player));

    player->width = 32;
    player->height = 32;

    // Three fields in the vector so we can take advantage of hardware-acceleration
    player->position = calloc(3, sizeof(int32));
    player->velocity = calloc(3, sizeof(int32));
    
    player->position[0] = intToFixed(SCREEN_WIDTH / 2, RESOLUTION);
    player->position[1] = intToFixed(SCREEN_HEIGHT - player->height, RESOLUTION);

    player->speed = intToFixed(5, RESOLUTION);

    return player;
}

void movePlayer(Player* player)
{
    if (player->velocity[0] != 0 || player->velocity[1] != 0) // The velocity is non-zero
    {
        normalizef32(player->velocity);
        //iprintf("Normalized: (%i, %i)\n", player->velocity[0], player->velocity[1]);
        //iprintf("Speed: %i\n", player->speed);
        scalarMultiply(player->speed, player->velocity);
        //iprintf("Scaled: (%i, %i)\n\n", player->velocity[0], player->velocity[1]);
        
        bool moved = false;

        // This is a bit of a mess :(
        // Note that in some places, the velocity is negative because we only care about the magnitude
        // In general, the pattern here is to check if the player's position plus their velocity
        // is in-bounds, along with a little polish of snapping them to the wall otherwise.
        // Maybe manually normalizing things would result in not only cleaner code, but faster code too.
        // Consider that test a TODO.
        if (player->velocity[0] > 0)
        {
            if (fixedToInt(player->position[0], RESOLUTION) <= SCREEN_WIDTH - player->width - fixedToInt(player->velocity[0], RESOLUTION))
            {
                player->position[0] += player->velocity[0];
                moved = true;
            }
            else if (fixedToInt(player->position[0], RESOLUTION) < SCREEN_WIDTH - player->width)
            {
                player->position[0] = intToFixed(SCREEN_WIDTH - player->width, RESOLUTION);
                moved = true;
            }
        }
        else if (player->velocity[0] < 0)
        {
            if (fixedToInt(player->position[0], RESOLUTION) >= fixedToInt(-player->velocity[0], RESOLUTION))
            {
                player->position[0] += player->velocity[0];
                moved = true;
            }
            else if (fixedToInt(player->position[0], RESOLUTION) > 0)
            {
                player->position[0] = 0;
                moved = true;
            }
        }

        if (player->velocity[1] > 0)
        {
            if (fixedToInt(player->position[1], RESOLUTION) <= SCREEN_HEIGHT - player->height - fixedToInt(player->velocity[1], RESOLUTION))
            {
                player->position[1] += player->velocity[1];
                moved = true;
            }
            else if (fixedToInt(player->position[1], RESOLUTION) < SCREEN_HEIGHT - player->height)
            {
                player->position[1] = intToFixed(SCREEN_HEIGHT - player->height, RESOLUTION);
                moved = true;
            }
        }
        else if (player->velocity[1] < 0)
        {
            if (fixedToInt(player->position[1], RESOLUTION) >= fixedToInt(-player->velocity[1], RESOLUTION))
            {
                player->position[1] += player->velocity[1];
                moved = true;
            }
            else if (fixedToInt(player->position[1], RESOLUTION) > 0)
            {
                player->position[1] = 0;
                moved = true;
            }
        }

        if (moved)
            NF_MoveSprite(0, 0, fixedToInt(player->position[0], RESOLUTION), fixedToInt(player->position[1], RESOLUTION));
    }

    return;
}

void playerControl(Player* player)
{
    scanKeys();
    uint32 keys = keysCurrent();

    if (keys & KEY_L)
        player->speed = intToFixed(2, RESOLUTION);
    else
        player->speed = intToFixed(5, RESOLUTION);

    if (keys & KEY_UP)
        player->velocity[1] = intToFixed(-1, RESOLUTION);
    else if (keys & KEY_DOWN)
        player->velocity[1] = intToFixed(1, RESOLUTION);
    else
        player->velocity[1] = 0;

    if (keys & KEY_LEFT)
        player->velocity[0] = intToFixed(-1, RESOLUTION);
    else if (keys & KEY_RIGHT)
        player->velocity[0] = intToFixed(1, RESOLUTION);
    else
        player->velocity[0] = 0;

    /*
    if (player->velocity[0] != 0 || player->velocity[1] != 0)
    {
        normalizef32(player->velocity);
        scalarMultiply(player->speed, player->velocity);
        
        if (testPosition(player->position, player->velocity, player->width, player->height))
        {
            addVector(player->position, player->velocity);
            NF_MoveSprite(0, 0, fixedToInt(player->position[0], RESOLUTION), fixedToInt(player->position[1], RESOLUTION));
        }
        else
        {
            
        }
    }
    */

    //iprintf("Velocity: (%i, %i)\n", player->velocity[0], player->velocity[1]);
    
    movePlayer(player);
    
    return;
}

void autoPilot(Player* player, Meteor** meteors, uint8 mCount)
{
    // Let's only init some stuff once
    static int32* target;
    static int32 targetBias;
    static int32 meteorBias; 
    static bool initialized = false;

    if (!initialized)
    {
        target    = malloc(sizeof(int32) * 2);
        target[0] = intToFixed(SCREEN_WIDTH / 2, RESOLUTION);
        target[1] = intToFixed(SCREEN_HEIGHT / 2, RESOLUTION);

        targetBias = floatToFixed(0.01f, RESOLUTION);
        meteorBias = intToFixed(15, RESOLUTION);
        
        initialized = true;
    }
    
    //static int32 falloff    = intToFixed(2, RESOLUTION);

    // Attractive force from the ship to the center of the screen;
    int32 shipToTarget[2];
    subVectorRef(target, player->position, shipToTarget);
    scalarMultiply(targetBias, shipToTarget);
    addVector(player->velocity, shipToTarget);

    int i = 0;
    int32 playerToMeteor[2];
    for (; i < mCount; i++)
    {
        subVectorRef(meteors[i]->position, player->position, playerToMeteor);
        scalarMultiply(intToFixed(-1, RESOLUTION), playerToMeteor); // Now it's pointing away
        int32 inverseFactor = mulf32(playerToMeteor[0], playerToMeteor[0]) + mulf32(playerToMeteor[1], playerToMeteor[1]);
        inverseFactor = divf32(meteorBias, inverseFactor);
        scalarMultiply(inverseFactor, playerToMeteor);

        addVector(player->velocity, playerToMeteor);
    }

    movePlayer(player);

    // We'll reset the player's velocity for the next frame:
    player->velocity[0] = 0;
    player->velocity[1] = 0;

    return;
}
