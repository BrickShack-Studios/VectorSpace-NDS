#include <stdio.h>
#include <nds.h>
#include <nf_lib.h>

// Broke flashcart mode
//#define BFM

#ifdef BFM
  #define ROOT_FOLDER "vectorSpaceResources"
#else
  #define ROOT_FOLDER "NITROFS"
#endif

#define RESOLUTION 12

const int32 unity[3] = {intToFixed(1, RESOLUTION), intToFixed(1, RESOLUTION), intToFixed(1, RESOLUTION)};

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

typedef struct Player
{
    int32* position;
    int32* velocity;
    
    uint8 width;
    uint8 height;

    int32 speed;
} Player;

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

typedef struct Meteor
{
    int32* position;

    uint8 width;
    uint8 height;
} Meteor;

Meteor* newMeteor()
{
    Meteor* meteor = calloc(1, sizeof(Meteor));

    meteor->width = 32;
    meteor->height = 32;

    meteor->position = calloc(3, sizeof(int32));

    return meteor;
}

void initNF()
{
    NF_Set2D(0, 0); // Set top screen to mode 0
    NF_Set2D(1, 0); // Set bottom screen to mode 0

    consoleDemoInit();
    iprintf("Loading...\n");
    swiWaitForVBlank();

    iprintf("Blanked\n");
    swiWaitForVBlank();

    NF_SetRootFolder(ROOT_FOLDER);

    iprintf("Set root folder\n");
    swiWaitForVBlank();

	NF_InitSpriteBuffers();
	NF_InitSpriteSys(0); // Init sprite system on top screen
	//NF_InitSpriteSys(1); // Init sprite system on bottom screen

    iprintf("Init sprite system\n");
    swiWaitForVBlank();
    
    NF_InitTiledBgBuffers();
    NF_InitTiledBgSys(0);
    //NF_InitTiledBgSys(1);

    iprintf("Set up background system\n");
    swiWaitForVBlank();
    
    return;
}

void loadSprites()
{
    NF_LoadSpriteGfx("sprites/player", 0, 32, 32); // Player is ID 0
    NF_LoadSpritePal("sprites/player", 0);

    NF_LoadSpriteGfx("sprites/smallMeteor", 1, 32, 32); // Meteor is ID 1
    NF_LoadSpritePal("sprites/smallMeteor", 1);

    iprintf("Loaded sprites\n");
    swiWaitForVBlank();

    NF_VramSpriteGfx(0, 0, 0, true);
    NF_VramSpritePal(0, 0, 0);

    NF_VramSpriteGfx(0, 1, 1, true);
    NF_VramSpritePal(0, 1, 1);

    iprintf("Moves sprites to VRAM\n");
    swiWaitForVBlank();
    
    return;
}

void playerMove(Player* player)
{
    scanKeys();
    uint32 keys = keysCurrent();    

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

    //iprintf("Velocity: (%i, %i)\n", player->velocity[0], player->velocity[1]);
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

int main(int argc, char **argv)
{
    srand(0); // same RNG for testing
    
    initNF();
    loadSprites();
    
    Player* player = newPlayer();
    Meteor* meteor = newMeteor();
    
    NF_CreateSprite(0, 0, 0, 0, fixedToInt(player->position[0], RESOLUTION), fixedToInt(player->position[1], RESOLUTION)); // Makes player sprite
    NF_CreateSprite(0, 1, 1, 1, meteor->position[0], meteor->position[1]); // Makes meteor sprite

    iprintf("Created sprite\n");
    swiWaitForVBlank();
    
    while(1)
    {
        playerMove(player);
        
        meteor->position[1] += 1;
        if (meteor->position[1] > SCREEN_HEIGHT)
        {
            meteor->position[1] = -meteor->height;
            meteor->position[0] = rand() % (SCREEN_WIDTH - meteor->width);
        }

        NF_MoveSprite(0, 1, meteor->position[0], meteor->position[1]);

        NF_SpriteOamSet(0);
		NF_SpriteOamSet(1);
        
		swiWaitForVBlank();

		oamUpdate(&oamMain);
		oamUpdate(&oamSub);
    }

    return 0;
}
