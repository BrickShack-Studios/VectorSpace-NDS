#include <nds.h>
#include <stdio.h>

#include "meteor.h"
#include "constants.h"

Meteor* newMeteor()
{
    Meteor* meteor = calloc(1, sizeof(Meteor));

    meteor->width = 32;
    meteor->height = 32;

    meteor->position = malloc(sizeof(int32) * 3);

    meteor->position[0] = intToFixed(rand() % (SCREEN_WIDTH - meteor->width), RESOLUTION);
    meteor->position[1] = intToFixed(rand() % SCREEN_HEIGHT, RESOLUTION);

    iprintf("Made new meteor at (%i, %i)\n", fixedToInt(meteor->position[0], RESOLUTION), fixedToInt(meteor->position[1], RESOLUTION));

    return meteor;
}
