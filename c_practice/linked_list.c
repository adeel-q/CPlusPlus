#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct node
{
    int value;
    struct node *next;
    int free;
} Node_t;

typedef struct ll
{
    Node_t* _node_pool;
    Node_t* head;
    size_t length;
    size_t size;
} LinkedList_t;

// Given input array , allocate memory and create the linked list
// Remember to free it!
int initLinkedList(LinkedList_t* theLinkedList, size_t n)
{
    theLinkedList->_node_pool = (Node_t*) malloc(n*sizeof(Node_t));
    
    if (theLinkedList->_node_pool)
    {
        theLinkedList->head = &theLinkedList->_node_pool[0];
        theLinkedList->length = 0;
        theLinkedList->size = n;

        // Init each node to default values
        for (size_t i = 0; i < theLinkedList->size; i++)
        {
            theLinkedList->_node_pool[i].value = 0;
            theLinkedList->_node_pool[i].next = 0;
            theLinkedList->_node_pool[i].free = 1;
        }

        return 1;
    }

    return 0;
}

void destroyLinkedList(LinkedList_t* theLinkedList)
{
    free(theLinkedList->_node_pool);
}

Node_t* getHead(LinkedList_t* theLinkedList)
{
    return theLinkedList->head;
}

size_t getLength(LinkedList_t* theLinkedList)
{
    return theLinkedList->length;
}

size_t getSize(LinkedList_t* theLinkedList)
{
    return theLinkedList->size;
}

size_t isFull(LinkedList_t* theLinkedList)
{
    return (theLinkedList->length == theLinkedList->size);
}

size_t isEmpty(LinkedList_t* theLinkedList)
{
    return (theLinkedList->length == 0);
}

// Appends element to the end of the linked list. NOTE: This does not insert in between! That would be a different method
// TODO: Convert to insert (appends would just be insert(atLength))
int addElement(LinkedList_t* theLinkedList, int element)
{
    printf("Adding element %i \n", element);
    if (!isFull(theLinkedList))
    {
        // If adding the first element in the empty list
        if (isEmpty(theLinkedList))
        {
            theLinkedList->head = &theLinkedList->_node_pool[0];
            theLinkedList->head->value = element;
            theLinkedList->head->free = 0; // Mark in-use
            theLinkedList->head->next = 0;
            theLinkedList->length += 1;
        }
        else
        {
            // Allocate Node_t from node pool
            int found = 0;
            size_t i =0;
            while( (theLinkedList->_node_pool[i].free == 0) && (i < theLinkedList->size) )
            {
                i++;
            }
            assert(theLinkedList->_node_pool[i].free == 1); // This assertion should always be true! If this fails, then that means array was *not* full but we did not find a free node.
            // Mark allocated
            Node_t* targetNode = &theLinkedList->_node_pool[i];
            targetNode->value = element; // Will be null if appending
            targetNode->free = 0; // Mark in use

            // Link in this node with the list:
            // Find node right before insertion point
            Node_t* pt = theLinkedList->head;
            while(pt->next) // While this element has a next pointer, go to the next pointer. If inserting, different criteria (would next while pt->next == elmement)
            {
                pt = pt->next;
            }
            // Loop has broken, we assume pt->next is null
            assert(0 == pt->next);
            // If we reached the end of list, this case is already handled because isFull will be true, and we won't be here
            // If we have reached end of list-1 , this means we have one available spot left
            theLinkedList->length += 1;
            assert(theLinkedList->length <= theLinkedList->size);
            // Populate node
            targetNode->next = pt->next; // Will be null if appending
            pt->next = targetNode;
            // We dont have to worry about the next pointer but we could point it back to head?
        }
        assert (!((theLinkedList->length -1) < 0));
        // Return position at which element was added (SHOULD NEVER BE NEGATIVE)
        return theLinkedList->length -1;
    }
    printf("List overflow!\n", element);
    return -1; // Could not add
}

// Delete first occurence of the element and return the position that was purged
// Return -1 if failed to find element to delete
int deleteElement(LinkedList_t* theLinkedList, int element)
{
    printf("Remove element %i \n", element);

    if (isEmpty(theLinkedList))
    {
        return -1;
    }

    Node_t* pt = theLinkedList->head;
    size_t i = 0;
    
    // [targetNode] -> null
    if (theLinkedList->length == 1 && theLinkedList->head->value == element)
    {
        theLinkedList->head = theLinkedList->_node_pool; // Empty linked list starts at the top of the pool
        theLinkedList->head->next = 0;
        theLinkedList->head->free = 1;
        theLinkedList->length -= 1;
        return 0;
    }
    // [targetNode(head)] -> [node2] -> [node3]
    else if (theLinkedList->head->value == element)
    {
        
        // Mark current head as free
        theLinkedList->head->free = 1;
        // Move head element to the next one
        theLinkedList->head = theLinkedList->head->next;
        theLinkedList->length -= 1;
        return 0;
    }
    // [pt] -> [targetNode] -> [node2] -> null
    // [pt] -> [targetNode] -> null
    else
    {
        int found = 0;
        // Search for node right before target node
        for (i =0; (i < theLinkedList->length) && (!found); i++)
        {
            if ((pt->next) && (pt->next->value == element))
            {
                found = 1; // Breaks out of loop
            }
            else
            {
                pt = pt->next;
            }
        }
        if (found)
        {
            // Delete node (edge cases)
            Node_t * targetNode = pt->next;
            pt->next = targetNode->next;
            targetNode->free = 1; // Mark as free node
            targetNode->next = 0;
            theLinkedList->length -= 1;
            // Return position where we found the element
            return i;
        }
        printf("Element %i not in linked list\n", element);
        return -1; // Did not find element
    }
}

void deleteWithoutHead(Node_t* nodeToDelete)
{
    // [?] -> [node1] ->[nodeToDelete] -> NULL
    // [?] -> [node1] ->[nodeToDelete] -> [node2]
    if(nodeToDelete)
    {
        if (!nodeToDelete->next)
        {
            // Can't delete need head
            return;
        }
        nodeToDelete->value = nodeToDelete->next->value; // Copy value of the next node into current node. In effect, we do not update links but we update the value
        nodeToDelete->next->free = 1; // Mark as free

    }
}

void displayLinkedList(LinkedList_t* theLinkedList)
{
    Node_t* node = theLinkedList->head;
    for (size_t i = 0; i < theLinkedList->size; i++)
    {
        if (node)
        {
            if(!node->free)
            {
                printf("{[@%x] [%i] next[@%x]} -> ", node, node->value, node->next);
            } 
            node = node->next;
        }
    }
    printf("\n\n");

}
void displayLinkedListInfo(LinkedList_t* theLinkedList)
{
    printf("size=[%lu]\n", theLinkedList->size);
    printf("length=[%lu]\n", theLinkedList->length);
    printf("_node_pool=[@%x]\n", &theLinkedList->_node_pool);
    printf("head=[%x]\n\n", theLinkedList->head);
    printf("\n");
}
void displayAllocated(LinkedList_t* theLinkedList)
{
    displayLinkedListInfo(theLinkedList);

    Node_t* node = theLinkedList->_node_pool;
    for (size_t i = 0; i < theLinkedList->size; i++)
    {
        printf("{[@%x] [%i] next[@%x]} ; ", &theLinkedList->_node_pool[i], theLinkedList->_node_pool[i].value, theLinkedList->_node_pool[i].next);
    }
    printf("\n");
}

// Testing linked list operations
int main(void)
{
    LinkedList_t ll;
    LinkedList_t* theLinkedList = &ll;// (LinkedList_t*)malloc(1*sizeof(LinkedList_t));


    // Create the linked list with the static array
    int success = initLinkedList(theLinkedList, 3);
    if (!success)
    {
        printf("Linked list initialization failed!\n");
    }
    printf("Initial list:\n");
    displayLinkedList(theLinkedList);
    // Do some operations
    // Add to linked list
    addElement(theLinkedList, 1);
    displayLinkedList(theLinkedList);

    deleteElement(theLinkedList, 2);
    displayLinkedList(theLinkedList);

    deleteElement(theLinkedList, 1);
    displayLinkedList(theLinkedList);

    addElement(theLinkedList, 1);
    displayLinkedList(theLinkedList);

    addElement(theLinkedList, 2);
    displayLinkedList(theLinkedList);

    // Remove first element
    deleteElement(theLinkedList, 1);
    displayLinkedList(theLinkedList);

    // Saturate size
    addElement(theLinkedList, 3);
    displayLinkedList(theLinkedList);
    addElement(theLinkedList, 4);
    displayLinkedList(theLinkedList);
    addElement(theLinkedList, 5);
    displayLinkedList(theLinkedList);

    displayAllocated(theLinkedList);
    
    destroyLinkedList(theLinkedList);
    
    return 0;

}