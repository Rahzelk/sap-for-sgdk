#include "sap.h"

//#define DEBUG
//#define DEBUG_EDGE_SORT

//maintained list of edges
EdgeList edgeList; 

void SAP_insertEntity(Entity* entity)
{
    if (edgeList.edgeCount + 2 > MAX_EDGES) 
    {
        #ifdef DEBUG
            KLog("Error: Too many edges to sort");
        #endif
        return;
    }

    // create new edge
    Edge newEdge;
    newEdge.entity = entity;

    // insert the entity two egdes of the Entity (left and right)
    for (u8 i = 0; i < 2; i++) 
    {
        newEdge.isLeft = i;  // 0 = left, 1 = right
        
        // newEdge position
        u16 newPosition = (newEdge.isLeft) ? newEdge.entity->currentBounds.min.x : newEdge.entity->currentBounds.max.x;
        
        // find where to insert this edge
        u8 insertIndex = edgeList.edgeCount;
        for (u8 j = 0; j < edgeList.edgeCount; j++) 
        {
            // compare newEdge position and the current fetched position
            u16 currentPosition = (edgeList.edges[j].isLeft) ? edgeList.edges[j].entity->currentBounds.min.x : edgeList.edges[j].entity->currentBounds.max.x;
            
            if (newPosition < currentPosition) 
            {
                insertIndex = j;
                break;
            }
        }
        
        // shift edges 
        for (u8 j = edgeList.edgeCount; j > insertIndex; j--) 
        {
            edgeList.edges[j] = edgeList.edges[j - 1];
        }

        // insert newEdge
        edgeList.edges[insertIndex] = newEdge;
        edgeList.edgeCount++;
    }
}



void SAP_init()
{
    edgeList.edgeCount = 0; 
    sweepCount = 0;
}

void SAP_sort() 
{
    for (u8 i = 1; i < edgeList.edgeCount; i++)
    {   
        Edge currentEdge = edgeList.edges[i];
        u16 currentX = currentEdge.isLeft 
            ? currentEdge.entity->currentBounds.min.x 
            : currentEdge.entity->currentBounds.max.x;

        s8 j = i - 1;

        // shift greater Edges
        while (j >= 0)
        {
            Edge* prevEdge = &edgeList.edges[(u8)j];
            u16 prevX = prevEdge->isLeft 
                ? prevEdge->entity->currentBounds.min.x 
                : prevEdge->entity->currentBounds.max.x;

            if (prevX <= currentX) break;

            // current edge is shifted to the right
            edgeList.edges[j + 1] = *prevEdge;
            j--;
        }

        // currentEdge is at the good place
        edgeList.edges[j + 1] = currentEdge;
    }

    #ifdef DEBUG_EDGE_SORT
        logEdgesList();
    #endif
}



void SAP_sweep() 
{
    if(sweepCount++ == SORT_FREQUENCY)
    {
        sweepCount = 0;
        SAP_sort();
    } 

    // this will hold the entities that are currently touching each other
    Entity* touching[MAX_ENTITY_TOUCHING];
    u8 touchingCount = 0;

    for (u8 i = 0; i < edgeList.edgeCount; i++)
    {
        Edge* current = &edgeList.edges[i];
        
        if (current->isLeft)
        {
            // check touching first before adding to the touching list
            for (u8 j = 0; j < touchingCount; j++) 
            {
                if( current->entity->currentBounds.min.y < touching[j]->currentBounds.max.y 
                && current->entity->currentBounds.max.y > touching[j]->currentBounds.min.y )
                {

                    #ifdef DEBUG
                        KLog_U2("Collision ett1 currentBounds x : ", current->entity->currentBounds.min.x, " ett2 currentBounds x : " , touching[j]->currentBounds.min.x);
                        KLog_U2("Collision ett1 currentBounds y: ", current->entity->currentBounds.min.y, " ett2 currentBounds y : " , touching[j]->currentBounds.min.y);
                    #endif

                    handleCollision(current->entity, touching[j]);
                }
            }

            // add to the touching list
            touching[touchingCount++] = current->entity;
        } 
        else 
        {
            // remove from the touching list
            for (u8 j = 0; j < touchingCount; j++) 
            {
                if (touching[j] == current->entity) {
                    touching[j] = touching[--touchingCount];
                    break;
                }
            }
        }
    }
}


void SAP_removeEntity(Entity* entity) 
{
    u8 i = 0, edgesDone =0;
    
    for(i = 0; i < edgeList.edgeCount && edgesDone <2; i++) 
    {    
        if (edgeList.edges[i].entity == entity) 
        {
            // shift the remaining edges
            for (u8 j = i; j < edgeList.edgeCount - 1; j++) 
            {
                edgeList.edges[j] = edgeList.edges[j + 1];
            }
            edgeList.edgeCount--; // one edge removed

            edgesDone++;
        } 
    }
}







void logEdgesList() 
{
    for (u8 i = 0; i < edgeList.edgeCount; i++) 
    {
        Edge edge = edgeList.edges[i];
        
        // Log the entity pointer and x coordinate
        KLog_U2("Edge ", i, " Entity Pointer: ", (u32)edge.entity);
        
        if(edge.entity != NULL) 
            KLog_U1("X Coordinate: ", edge.isLeft?(u32)edge.entity->currentBounds.min.x:(u32)edge.entity->currentBounds.max.x);
        
        // Log whether this is the left edge
        KLog_U1("Is Left: ", (u32)edge.isLeft);
    }
}
