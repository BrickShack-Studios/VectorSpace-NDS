#include <stdio.h>
#include <nds.h>
#include <nf_lib.h>

#include "constants.h"
#include "vector.h"
#include "player.h"
#include "meteor.h"

// Broke flashcart mode
//#define BFM

#ifdef BFM
  #define ROOT_FOLDER "vectorSpaceResources"
#else
  #define ROOT_FOLDER "NITROFS"
#endif

const uint8 DEMO_METEORS = 6;

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

void loadBg()
{
    NF_LoadTiledBg("bg/space", "space", 256, 256);
    NF_CreateTiledBg(0, 0, "space");
    
    return;
}

int main(int argc, char **argv)
{
    srand(0); // same RNG for testing
    
    initNF();
    loadSprites();
    loadBg();
    iprintf("Loaded background\n");
    
    Player* player = newPlayer();
    // Meteor* meteor = newMeteor();

    // Make player sprite
    NF_CreateSprite(0, 0, 0, 0, fixedToInt(player->position[0], RESOLUTION), fixedToInt(player->position[1], RESOLUTION));
    //NF_CreateSprite(0, 1, 1, 1, meteor->position[0], meteor->position[1]); // Makes meteor sprite

    // Make meteor sprites:
    Meteor** meteors = malloc(sizeof(Meteor*) * DEMO_METEORS);
    int i = 0;
    for (i = 0; i < DEMO_METEORS; i++)
    {
        meteors[i] = newMeteor();
        NF_CreateSprite(0, 1 + i, 1, 1, fixedToInt(meteors[i]->position[0], RESOLUTION), fixedToInt(meteors[i]->position[1], RESOLUTION));
    }

    iprintf("Created sprite\n");
    swiWaitForVBlank();

    s16 bgPosition[2] = {0, 0};
    
    while(1)
    {
        //playerMove(player);

        //iprintf("(%i, %i)\t(%i, %i)\t(%i, %i)\n", meteors[0]->position[0], meteors[0]->position[1], meteors[1]->position[0], meteors[1]->position[1], meteors[2]->position[0], meteors[2]->position[1]);

        for (i = 0; i < DEMO_METEORS; i++)
        {
            meteors[i]->position[1] += intToFixed(1, RESOLUTION);
            if (fixedToInt(meteors[i]->position[1], RESOLUTION) > SCREEN_HEIGHT)
            {
                meteors[i]->position[1] = intToFixed(-meteors[i]->height, RESOLUTION);
                meteors[i]->position[0] = intToFixed(rand() % (SCREEN_WIDTH - meteors[i]->width), RESOLUTION);
            }

            NF_MoveSprite(0, 1 + i, fixedToInt(meteors[i]->position[0], RESOLUTION), fixedToInt(meteors[i]->position[1], RESOLUTION));
        }

        bgPosition[1] -= 1;
        NF_ScrollBg(0, 0, bgPosition[0], bgPosition[1]);

        autoPilot(player, meteors, DEMO_METEORS);
        
        NF_SpriteOamSet(0);
		NF_SpriteOamSet(1);
        
		swiWaitForVBlank();

		oamUpdate(&oamMain);
		oamUpdate(&oamSub);
    }

    return 0;
}
