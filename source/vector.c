#include <nds.h>

#include "vector.h"

// Adds b into a
void addVector(int32* a, int32* b)
{
    a[0] += b[0];
    a[1] += b[1];

    return;
}

void scalarMultiply(int32 s, int32* v)
{
    v[0] = mulf32(s, v[0]);
    v[1] = mulf32(s, v[1]);

    return;
}

void subVectorRef(int32* a, int32* b, int32* result)
{
    result[0] = a[0] - b[0];
    result[1] = a[1] - b[1];

    return;
}

bool onScreen(int32* position, int32 width, int32 height)
{
    return !(position[0] < 0 ||
             position[0] + width >= SCREEN_WIDTH ||
             position[1] < 0 ||
             position[1] + height >= SCREEN_HEIGHT);
}

bool testPosition(int32* position, int32* displacement, int32 width, int32 height)
{
    int32 tmp[2];

    tmp[0] = position[0] + displacement[0];
    tmp[1] = position[1] + displacement[1];

    return onScreen(tmp, width, height);
}
