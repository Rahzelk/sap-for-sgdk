#include <genesis.h>
#include "resources.h"
#include "sap.h"


// simply comment this to test without Sweep And Prune
#define SAP_TEST

// adjust the number of sprites for a custom benchmark
#define NB_SPRITES 40


// will store our sprites 
Entity listEtt[NB_SPRITES];

	
//small function to get a number between -2 and  2, but not 0
s8 randTwo() {
    s8 randomNum;
    do {
        randomNum = (random() % 3) - 2; 
    } while (randomNum == 0);           
    return randomNum;
}

s8 rand(u8 max) {
    s8 randomNum;
    do {
        randomNum = (random() % max) - (max-1); 
    } while (randomNum == 0);           
    return randomNum;
}

// classic check for Axis Aligned Bounded Box collision
bool checkAABBCollision(AABB a, AABB b) {
    return (a.min.x < b.max.x && a.max.x > b.min.x &&
            a.min.y < b.max.y && a.max.y > b.min.y);
}

// just doing a rebound when collisioning
void handleCollision(Entity * ett1, Entity * ett2)
{
		//ett2->move.x = 	-ett2->move.x;
		ett2->move.y = 	(-1*ett2->move.y);
		//ett1->move.x = 	-ett1->move.x;
		ett1->move.y = (-1*ett1->move.y);
}

void createDonut(u8 index, u8 PAL, u8 type, Vect2D_s16 move)
{
	listEtt[index].sprite  = SPR_addSprite(&donut, (random() % 15)*16,  (random() % 10)*16, TILE_ATTR(PAL, 1, FALSE, FALSE));
	listEtt[index].currentBounds = (AABB){
										{listEtt[index].sprite->x, listEtt[index].sprite->y},
										{listEtt[index].sprite->x + 16, listEtt[index].sprite->y+16}
									};
	listEtt[index].type = type;
	listEtt[index].move = move;

	#ifdef	SAP_TEST
		SAP_insertEntity(&listEtt[index]);
	#endif
}

void initSpriteList()
{
	createDonut(0, PAL3, ENTITY_COLLIDE, (Vect2D_s16){2,0});

	for (u8 i =1; i < NB_SPRITES; i++)
	{
		createDonut(i, PAL0, ENTITY_NOT_COLLIDE, (Vect2D_s16){randTwo(),randTwo()});
	}
}

void updateSpritePosition()
{
	for (u8 i =0; i < NB_SPRITES; i++)
	{
		s16 newPosX = SPR_getPositionX(listEtt[i].sprite) + listEtt[i].move.x;
		s16 newPosY = SPR_getPositionY(listEtt[i].sprite) + listEtt[i].move.y;

		if(newPosX < 0)
		{ 
			newPosX = 0;
			listEtt[i].move.x = -listEtt[i].move.x;
		}
		if(newPosY < 0)
		{ 
			newPosY = 0;
			listEtt[i].move.y = -listEtt[i].move.y ;
		}
		if(newPosX >= 304)
		{
			newPosX = 304;
			listEtt[i].move.x = -listEtt[i].move.x;
		}
		if(newPosY >= 208)
		{
			newPosY = 208;
			listEtt[i].move.y = -listEtt[i].move.y;
		}

		SPR_setPosition(listEtt[i].sprite, newPosX, newPosY);
		listEtt[i].currentBounds = (AABB){
											{listEtt[i].sprite->x, listEtt[i].sprite->y},
											{listEtt[i].sprite->x + 16, listEtt[i].sprite->y+16}
										};		
		
	}		
}


void checkCollisions()
{
	#ifndef	SAP_TEST
		for (u8 i =0; i < NB_SPRITES; i++)
		{
			if(listEtt[i].type)
			{
				for(u8 j=0; j<NB_SPRITES; j++)
				{
					if(j!=i && checkAABBCollision(listEtt[i].currentBounds, listEtt[j].currentBounds))
					{
						handleCollision(&listEtt[i], &listEtt[j]);
					}
				}
			}
		}
	#endif	

	#ifdef	SAP_TEST
		SAP_sweep();
	#endif
}


	
int main(bool resetType) 
{ 
	
	//Soft resets don't clear RAM, this can bring some bugs so we hard reset every time we detect a soft reset
	if (!resetType)
	    SYS_hardReset();

    VDP_setScreenHeight224();
    VDP_setScreenWidth320();

	SPR_init();
    

	#ifdef	SAP_TEST
		SAP_init();
	#endif

	initSpriteList();

    SPR_update();
    SYS_doVBlankProcess();


    while(1)
    {      
		
		updateSpritePosition();
		checkCollisions();

		SPR_update();
		KLog_U1 ("CPU LOAD :", SYS_getCPULoad());

        SYS_doVBlankProcess();        
    }
}
