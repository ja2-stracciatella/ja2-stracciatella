//***********************************************
//
// Filename : Container.h
//
// Purpose : prototypes for the container file
//
// Modification History : 25 Nov 96 Creation
//
//***********************************************

#ifndef _CONTAINER_H
#define _CONTAINER_H

//***********************************************
//
//   Includes
//
//
//***********************************************

#include "Types.h"

//***********************************************
//
//  Defines and typedefs
//
//***********************************************
#define ORDLIST_ERROR                 -1
#define ORDLIST_EQUAL                 0
#define ORDLIST_LEFT_LESS             1
#define ORDLIST_RIGHT_LESS            2

typedef void * HCONTAINER;
typedef HCONTAINER HSTACK;
typedef HCONTAINER HQUEUE;
typedef HCONTAINER HLIST;
typedef HCONTAINER HORDLIST;

//***********************************************
//
// Function Prototypes
//
//***********************************************
#ifdef __cplusplus
extern "C" {
#endif

// call these functions to initialize and shutdown the debug messages for
// containers
extern void InitializeContainers(void);
extern void ShutdownContainers(void);

// Stack Functions
// CreateStack(estimated number of items in stack, size of each item
// Push(handle to container returned from CreateStack, data to be passed in (must be void *)
//  : returns handle to new stack
// Pop(handle to container returned from CreateStack, data to be passed in (must be void *)
//  : returns BOOLEAN
// DeleteStack deletes the stack container
// StackSize returns size of stack

extern HSTACK CreateStack(UINT32 num_of_elem , UINT32 siz_of_each);
extern HSTACK Push(HSTACK hStack, void *data);
extern BOOLEAN Pop(HSTACK hStack, void *data);
extern UINT32 StackSize(HSTACK hStack);
extern BOOLEAN DeleteStack(HSTACK hStack);
extern BOOLEAN PeekStack(HSTACK hStack, void *data);

// Queue Functions
// CreateQueue(estimated number of items in queue, size of each item
// AddtoQueue(handle to container returned from CreateQueue, data to be passed in (must be void *))
// : returns handle to queue
// RemfromQueue(handle to container returned from CreateQueue, variable where data is stored (must be void *))
// : returns BOOLEAN
// PeekQueue(handle to the queue, variable where peeked data is stored). Item is not deleted.
// : returns BOOLEAN
// QueueSize(handle to the queue) returns the queue size
// DeleteQueue(handle to container) Delete the queue container
// : returns BOOLEAN

extern HQUEUE  CreateQueue(UINT32 num_of_elem, UINT32 siz_of_each);
extern HQUEUE AddtoQueue(HQUEUE hQueue, void *data);
extern BOOLEAN RemfromQueue(HQUEUE hQueue,void *data);
extern BOOLEAN PeekQueue(HQUEUE hQueue, void *data);
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
extern BOOLEAN SwapListNode(HLIST hList, void *pdata, UINT32 uiPos);
extern BOOLEAN StoreListNode(HLIST hList, void *pdata, UINT32 uiPos);

// Ordered List Functions
// CreateOrdList(estimated number of items in ordered list, size of each item,
//    pointer to a compare function that returns info on whether the data in the ordered stack
//    is < or > the new data to be added into the ordered list.
// AddtoOrdList(handle to container returned from CreateOrdList, data to be passed in (must be void *)
// RemfromOrdList(handle to container returned from CreateList, variable where data is stored (must be void *)
//          position where data is to be deleted (0...sizeof(list)-1)
// PeekOrdList(handle to the list, variable where peeked data is stored). Item is not deleted.
//          position where data is to be peeked (0...sizeof(list)-1)
// OrdListSize(handle to the list) returns the ordered list size
// DeleteOrdList(handle to the list) Delete the ordered list container

extern HLIST  CreateOrdList(UINT32 num_of_elem, UINT32 siz_of_each, INT8 (*compare)(void *,void *, UINT32));
extern HLIST AddtoOrdList(HLIST hList, void *data);
extern BOOLEAN RemfromOrdList(HLIST hList,void *data, UINT32 position);
extern BOOLEAN PeekOrdList(HLIST hList, void *data, UINT32 position);
extern UINT32  OrdListSize(HLIST hList);
extern BOOLEAN DeleteOrdList(HLIST hList);

#ifdef __cplusplus
}
#endif

#endif
