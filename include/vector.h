#ifndef VECTOR_H
#define VECTOR_H

#include <nds.h>

void addVector(int32* a, int32* b);

void scalarMultiply(int32 s, int32* v);

/**
   Calculates a - b, but saves the result in result instead of returning it.
 */
void subVectorRef(int32* a, int32* b, int32* result);

/**
   Checks if a point is on the screen, given some bounds.
 */
bool onScreen(int32* position, int32 width, int32 height);

/** 
    Checks if position+displacement is in the screen, given the bounds of the object.
*/
bool testPosition(int32* position, int32* displacement, int32 width, int32 height);

#endif
