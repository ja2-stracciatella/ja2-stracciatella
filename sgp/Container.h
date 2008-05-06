//***********************************************
//
// Filename : Container.h
//
// Purpose : prototypes for the container file
//
// Modification History : 25 Nov 96 Creation
//
//***********************************************

#ifndef CONTAINER_H
#define CONTAINER_H

#include "Types.h"

typedef struct QueueHeader* HQUEUE;
typedef struct ListHeader*  HLIST;

// Queue Functions
// CreateQueue(estimated number of items in queue, size of each item
// AddtoQueue(handle to container returned from CreateQueue, data to be passed in (must be void *))
// : returns handle to queue
// RemfromQueue(handle to container returned from CreateQueue, variable where data is stored (must be void *))
// : returns BOOLEAN
// QueueSize(handle to the queue) returns the queue size
// DeleteQueue(handle to container) Delete the queue container
// : returns BOOLEAN

extern HQUEUE  CreateQueue(UINT32 num_of_elem, UINT32 siz_of_each);
extern HQUEUE AddtoQueue(HQUEUE hQueue, void *data);
extern BOOLEAN RemfromQueue(HQUEUE hQueue,void *data);
extern UINT32  QueueSize(HQUEUE hQueue);
extern BOOLEAN DeleteQueue(HQUEUE hQueue);

// List Functions
// CreateList(estimated number of items in queue, size of each item
// AddtoList(handle to container returned from CreateQueue, data to be passed in (must be void *)
//          position where data is to be added (0...sizeof(list))
// : returns handle to new list
// RemfromList(handle to container returned from CreateList, variable where data is stored (must be void *)
//          position where data is to be deleted (0...sizeof(list)-1)
// PeekList(handle to the list, variable where peeked data is stored). Item is not deleted.
//          position where data is to be peeked (0...sizeof(list)-1)
// ListSize(handle to the list) returns the list size
// DeleteList(handle to the list) Delete the list container

extern HLIST   CreateList(UINT32 num_of_elem, UINT32 siz_of_each);
extern HLIST   AddtoList(HLIST hList, void *data, UINT32 position);
extern BOOLEAN RemfromList(HLIST hList,void *data, UINT32 position);
extern BOOLEAN PeekList(HLIST hList, void *data, UINT32 position);
extern UINT32  ListSize(HLIST hList);
extern BOOLEAN DeleteList(HLIST hList);

#endif
