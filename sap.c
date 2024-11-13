/*******************************************************
 * File    : sap.c
 * Author  : Rahzelk (raz8y@hotmail.com)
 * Date    : 13/11/2024
 * Version : 0.9
 * 
 * Purpose  : This is a Sweep and Prune algorithm for collision detection.
 *           Developped for a Megadrive homebrew game using  SGDK devkit.
 *           This Sweep And Prune works on the X axis.
 *           Note that a quick check on the Y axis is done before calling a custom function
 *           (then you can do narrow phase or direct handling of the collision)
 * 
 *          thanks goes to https://leanrada.com/notes/sweep-and-prune/
 *          a must read for Sweep And Prune understanding !
 * 
 * Usage   : you'll see mainly a struct like Entity, wich act as an object-kind 
 *			 to store things like sprit, position and at least an AABB "currentBounds" property 
  * 
 *			 Note that currentBounds is a real-time updated hitbox surrounding any entity.
 *           It must be the "current" min.x, min.y, max.x, max.y  of the entity.
 *           If you have a large hitbox, you may use this SAP as a broad phase before narrowing
 * 
 *          1/ first, call SAP_init();
 *          2/ then load your SAP with your items using SAP_insertEntity(myEntity)
 *          3/ update all the entities' currentBounds
 *                this SAP algorithme assume to update entities positions before checking collisions
 *          4/ when ready to check collision, call SAP_sweep();
 *            Note that you may do this once per vblank interval
 *            Note that you'll need to code a behavior function to handle collisions
 *              (see handleCollision - but you may want to put this in another .c file)
 *          5/ to remove any Entity from the SAP, simply use SAP_removeEntity
 *             e.g. : when your entity is destroyed or not anymore on screen
 * 
 * Change history :
 *   - [13/11/2024] Rahzelk : first publication 
 * 
 * Licence : this code is distributed under the MIT license
 *
 *******************************************************/

#include <genesis.h>



//The maximum number of edges that can be take into account
// note : 1 entity = 2 edges so 72 edges = 36 entities max.
#define MAX_EDGES 72 

//The maximum number of entity that can be touching each other at the same time
// note : this concerns entity on the same X axis, but probably distributed on the Y axis.
#define MAX_ENTITY_TOUCHING 12 



// simple AABB ( Axis Aligned Bounding Box )
typedef struct AABB{
	Vect2D_u16 min;
	Vect2D_u16 max;
}AABB;


// represent any collidable objects
typedef struct Entity
{
    Sprite * sprite;
    Vect2D_u16 pos;       
    AABB currentBounds;
} Entity;



/**
 * \brief Represents an edge in the Sweep and Prune (SAP) algorithm.
 *
 * This struct contains a pointer to the entity 
 * associated with the edge and a flag indicating whether the edge is the left edge of the entity.
 */
typedef struct Edge {
    Entity* entity;
    u8 isLeft; // TRUE if this edge is the left edge of the entity
} Edge;


/**
 * \brief Represents a list of edges for the Sweep and Prune (SAP) algorithm.
 *
 * This struct contains an array of edges and a count of the number of edges in the list.
 * The maximum number of edges that can be stored in the list is defined by MAX_EDGES.
 */
typedef struct EdgeList{
    Edge edges[MAX_EDGES]; 
    u8 edgeCount;          
} EdgeList;



/**
 * \brief Initializes the Sweep and Prune (SAP) algorithm.
 *
 * This function initializes the edge list for the SAP algorithm.
 * It sets the edge count to 0, indicating that no edges have been added yet.
 *
 */
void SAP_init();



/**
* \brief Inserts an entity into the edge list for the Sweep and Prune algorithm.
* This function creates two edges for the given entity 
* (one for the left boundary and one for the right boundary) and inserts them into the edge list.
* 
* If the edge list is already full (i.e., the maximum number of edges has been reached), 
* the function returns without inserting the entity.
*
* \param entity A pointer to the entity to be inserted.
*/
void SAP_insertEntity(Entity* entity);


/**
 * \brief Performs the sweep phase of the Sweep and Prune (SAP) algorithm.
 *
 * This function performs the sweep phase of the SAP algorithm. It iterates through the sorted edge list,
 * checking for collisions between entities and updating the touching entities list.
 * 
 * \note collision is checked only after  y-coordinates overlap confirmation
 *
 */
void SAP_sweep();


/**
 * \brief Performs anything you may have to do when a collision is detected between two entities
 *
 *
* \param ett1 A pointer to the first entity implied
* \param ett2 A pointer to the second entity implied
*
*/
void handleCollision(Entity * ett1, Entity * ett2);


/**
 * \brief Sorts the edges in the edge list based on their x-coordinates.
 *
 * This function implements the insertion sort algorithm to sort the edges in the edge list.
 * The edges are sorted based on their x-coordinates
 * sorting is ascending from  lower to higher x-coordinates.
 *
 * \note This function is called internally by the SAP_sweep() function.
 *
 */
void SAP_sort();


/**
* \brief Removes an entity from the edge list for the Sweep and Prune algorithm.
* This function removes the two edges associated with the given entity from the edge list.
* The edges are identified by their entity pointer and the function shifts the remaining edges to fill the gap.
*
* \param entity A pointer to the entity to be removed.
*
* \note The function does not check if the entity is in fact present in the edge list
* \note The function does not deallocate memory for the entity or any other associated resources.
* It only removes the entity from the edge list for collision detection.
*/
void SAP_removeEntity(Entity* entity);


/**
 * \brief Logs the details of all edges in the edge list.
 *
 * This function iterates through the edge list and logs the details of each edge.
 * The details include the entity pointer, x-coordinate, and whether the edge is a left edge.
 *
 * \note This function is used for debugging purposes only.
 */
void logEdgesList();





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

    // insert two edges for the entity
    for(u8 i = 0; i < 2; i++) 
    {
        newEdge.isLeft = i; // will be alternately 0 or 1 == true or false
        edgeList.edges[edgeList.edgeCount++] = newEdge;    
    }
}



void SAP_init()
{
    edgeList.edgeCount = 0; 
}



void handleCollision(Entity * ett1, Entity * ett2)
{
    //do your collision stuff
}


void SAP_sort() 
{
    for (u8 i = 1; i < edgeList.edgeCount; i++)
    {   
        Edge currentEdge = edgeList.edges[i];

        s8 j = i - 1;
        u8 k = i;

        // compute X corresponding to the current edge
        u16 currentX = currentEdge.isLeft ? currentEdge.entity->currentBounds.min.x 
                                            : currentEdge.entity->currentBounds.max.x;

        // shift greater edges to the right
        while (j >= 0) 
        {   
            u8 c = (u8)j;
            
            u16 edgeX = edgeList.edges[c].isLeft ? edgeList.edges[c].entity->currentBounds.min.x 
                                        : edgeList.edges[c].entity->currentBounds.max.x;

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

    #ifdef DEBUG_EDGE_SORT
        logEdgesList();
    #endif
}


void SAP_sweep() 
{
    SAP_sort(); // sort edges before sweeping
    
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