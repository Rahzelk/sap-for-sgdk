#include "sap.h"

#include "../inc/map.h"

//#define DEBUG
//#define DEBUG_EDGE_SORT


//maintained list of edges
EdgeList edgeList; 
s8 sweepCount;
bool sweepInProgress;


static inline s16 getEdgeValue(const Edge * edge)
{
    return edge->isLeft ? edge->entity->currentBounds.min.x : edge->entity->currentBounds.max.x;
}


void SAP_insertEntity(Entity* entity)
{
    if (edgeList.edgeCount + 2 > MAX_EDGES) 
    {
        #if DEBUG
            KLog("Error: Too many edges to sort");
        #endif
        return;
    }

    // create new edge
    Edge newEdge;
    newEdge.entity = entity;

    s16 newPosition;
    u8 edgeCount, insertIndex, k;

    // insert the entity two egdes of the Entity (left and right)
    for (u8 i = 0, j=0; i < 2; i++) 
    {
        newEdge.isLeft = i;  // 0 = left, 1 = right
        
        // newEdge position
        newPosition = getEdgeValue(&newEdge);
        
        // find where to insert this edge
        edgeCount = edgeList.edgeCount;
        insertIndex = edgeCount;
        
        for (; j < edgeCount; j++) 
        {
            // compare newEdge position and the current fetched position
            if (newPosition < getEdgeValue(&edgeList.edges[j])) 
            {
                insertIndex = j;
                break;
            }
        }
        
        // shift edges 
        for (k = edgeCount; k > insertIndex; k--) 
        {
            edgeList.edges[k] = edgeList.edges[k - 1];
        }

        // insert newEdge
        edgeList.edges[insertIndex] = newEdge;
        edgeList.edgeCount++;
    }
}



void SAP_init()
{
    edgeList.edgeCount = 0; 
    sweepCount = SORT_FREQUENCY;
    sweepInProgress = FALSE;
}




void SAP_clean() 
{
    u8 writeIndex = 0;
    u8 edgeCount = edgeList.edgeCount;
    for (u8 i = 0; i < edgeCount; i++) 
    { 
        if (edgeList.edges[i].entity != NULL) 
        {
            edgeList.edges[writeIndex++] = edgeList.edges[i];
        }
    }
    edgeList.edgeCount = writeIndex; 
    
}


void SAP_sort() 
{
    SAP_clean();
    u8 edgeCount = edgeList.edgeCount;
    
    for (u8 i = 1; i < edgeCount; i++)
    {   
        if(!(i < edgeCount) )return; //last edge was a NULL, nothing more to sort
        
        Edge currentEdge = edgeList.edges[i];

        s8 j = i - 1;
        u8 k = i;

        // compute X corresponding to the current edge
        u16 currentX = getEdgeValue(&currentEdge);

        // shift greater edges to the right
        while (j >= 0) 
        {   
            u8 c = (u8)j;

            u16 edgeX = &edgeList.edges[c].entity ? getEdgeValue(&edgeList.edges[c]) : 0;

            if (edgeX > currentX)
            {
                edgeList.edges[c + 1] = edgeList.edges[c];
                k = j;
            }
            else
            {
                break; // no need to search further
            }
        
            j--;
        }

        // insert current edge
        edgeList.edges[k] = currentEdge;
    }

    #if SAP_DEBUG
        logEdgesList();
    #endif
}


void SAP_sweep() 
{
    if(--sweepCount < 1)
    {
        sweepCount = SORT_FREQUENCY;  
        SAP_sort();
    } 
    
    // this will hold the entities that are currently touching each other
    Entity** touching[MAX_ENTITY_TOUCHING];
    u8 touchingCount = 0;
    sweepInProgress = TRUE;

    #if DEBUG 
        KLog_U1("------------------- SAP_sweep start - edgeList.edgeCount",  edgeList.edgeCount);
    #endif

    for (u8 i = 0; i < edgeList.edgeCount; i++)
    {
        Edge* currentEdge = &edgeList.edges[i];
        Entity * currentEntity = currentEdge->entity;

        #if SAP_DEBUG 
            KLog_U2("---- i= ", i, " currentEdge->entity : ", currentEntity);
        #endif

        if (currentEdge && currentEdge->isLeft )
        {       
            
            #if SAP_DEBUG      
                KLog_U1("------- Left edge ! comparing currentEdge->entity :", currentEntity);
                KLog_U1("------- touchingCount:", touchingCount);
            #endif

            // check touching first before adding to the touching list
            for (u8 j = 0; j < touchingCount && currentEntity; j++) 
            {   
                #if SAP_DEBUG   
                    KLog_U3("---------- j= ", j, " *touching[j] : ", *touching[j], " currentEdge->entity : ",  currentEntity);                  
                #endif
                
                
                u8 collisionHappen = currentEntity->type || (*touching[j])->type;

                if( 
                 collisionHappen
                && currentEntity->currentBounds.min.y < (*touching[j])->currentBounds.max.y 
                && currentEntity->currentBounds.max.y > (*touching[j])->currentBounds.min.y )
                {
                    #if DEBUG   
                        KLog_U2("Collision ett1 : ", current->entity, " ett2 : " , (*touching[j]));
                        KLog_U2("Collision ett1 posx : ", current->entity->currentOuterHbBounds.min.x, " ett2 posx : " , (*touching[j])->currentOuterHbBounds.min.x);
                        KLog_U2("Collision ett1 posy: ", current->entity->currentOuterHbBounds.min.y, " ett2 posy : " , (*touching[j])->currentOuterHbBounds.min.y);
                    #endif

                    handleCollision(currentEntity, *touching[j]);

                    // adapt the touching table if touching[j] has been removed from SAP in between
                    if(!(*touching[j]))
                    {
                        #if DEBUG   
                            KLog_U1("---------- *touching[j] removed !! : ", touching[j]);
                        #endif

                        touching[j] = touching[--touchingCount];
                    }
                }
            }
            // add to the touching list except if current->entity has been removed from SAP in between
            if(currentEntity) 
            {
                touching[touchingCount++] =  &currentEdge->entity;

                #if DEBUG   
                    KLog_U1("---------- touching[touchingCount++] added in touching !! : ", current->entity);
                #endif                    
            }        
            else
            {
                #if DEBUG   
                    KLog_U1("---------- current->entity removed !! : ", current->entity);                   
                #endif
            }               
        } 
        else 
        { 
            #if DEBUG   
                KLog_U1("------- Right edge, to delete in touching : current->entity : ", current->entity);
            #endif

            // remove from the touching list
            for (u8 j = 0; j < touchingCount; j++) 
            {
                if (*touching[j] == currentEntity) 
                {
                    touching[j] = touching[--touchingCount];
                    break;
                }
            }
            
        }
    }
    sweepInProgress = FALSE;
}



void SAP_removeEntity(Entity* entity) 
{  
    for(u8 i = 0, edgesDone =0; i < edgeList.edgeCount && edgesDone <2;i++) 
    {    
        if (edgeList.edges[i].entity == entity) 
        {
            edgeList.edges[i].entity = NULL;
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
