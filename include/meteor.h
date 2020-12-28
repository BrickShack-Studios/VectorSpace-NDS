#ifndef METEOR_H
#define METEOR_H

#include <nds.h>

typedef struct Meteor
{
    int32* position;

    int16 width;
    int16 height;
} Meteor;

Meteor* newMeteor();

#endif
