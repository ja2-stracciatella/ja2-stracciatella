//*****************************************************************************
//
// Filename : Container.c
//
// Purpose : Function definition for the Container
//
// Modification History :
// 25 nov 96 TS creation
//
// 19 Dec 97 AM Replace all memcpy() with memmove(), since overlap possibilities
//							abound (and there were already bugs in do_copy() for ordered lists)
//							While the memcpy() was working in _DEBUG mode, it was failing
//							reproducibly in RELEASE mode whenever regions overlapped!
//							Having read the code, I strongly suggest that you DO NOT use this
//							stuff at all and write your own instead.  Tarun was no Carmack...
//																									- Alex Meduna
// 1998	KM Detached all references to this file from JA2 as it caused a lot of hard to debug
//         crashes.  The VOBJECT/VSURFACE lists are now self-maintained and no longer use the
//				 this crap.  DON'T USE THIS -- NO MATTER WHAT!!!
//*****************************************************************************

#include "Container.h"
#include "Debug.h"
#include "MemMan.h"
#include "Types.h"
#include <string.h>


typedef struct HeaderTag
{
	UINT32 uiTotal_items;
	UINT32 uiSiz_of_elem;
	UINT32 uiMax_size;
	UINT32 uiHead;
	UINT32 uiTail;

} QueueHeader , ListHeader;


//*****************************************************************************
//
// CreateQueue
//
// Parameter List : num_items - estimated number
//									of items in queue
//									siz_each - size of each item
// Return Value	NULL if unsuccesful
//							 pointer to allocated memory
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
HQUEUE CreateQueue(UINT32 uiNum_items, UINT32 uiSiz_each)
{
	UINT32 uiAmount;
	HQUEUE hQueue;
	QueueHeader *pQueue;

	// check to see if the queue has more than 1
	// element to be created and that the size > 1

	if ((uiNum_items > 0) && (uiSiz_each > 0))
		uiAmount = uiNum_items * uiSiz_each;
	else
	{
		DbgMessage(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "Requested queue items and size have to be >0");
			return NULL;
	}

		// allocate the queue memory
	if ((hQueue = MemAlloc(uiAmount + sizeof(QueueHeader))) == 0)
	{
			DbgMessage(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "Could not allocate queue container memory");
		return NULL;
		}

	pQueue = (QueueHeader *)hQueue;
	//initialize the queue structure

		pQueue->uiMax_size = uiAmount + sizeof(QueueHeader);
	pQueue->uiTotal_items = 0;
	pQueue->uiSiz_of_elem = uiSiz_each;
	pQueue->uiTail = pQueue->uiHead = sizeof(QueueHeader);

	// return the pointer to memory
	return hQueue;
}


//*****************************************************************************
//
// CreateList
//
// Parameter List : num_items - estimated number
//									of items in ordered list
//									siz_each - size of each item
// Return Value	NULL if unsuccesful
//							 pointer to allocated memory
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
HLIST CreateList(UINT32 uiNum_items, UINT32 uiSiz_each)
{
	UINT32 uiAmount;
	HLIST hList;
	ListHeader *pList;

	// check to see if the queue has more than 1
	// element to be created and that the size > 1

	if ((uiNum_items > 0) && (uiSiz_each > 0))
		uiAmount = uiNum_items * uiSiz_each;
	else
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Requested queue items and size have to be >0");
			return 0;
	}

		// allocate the list memory
	if ((hList = MemAlloc(uiAmount + sizeof(ListHeader))) == 0)
	{
			DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not allocate queue container memory");
		return 0;
		}

	pList = (ListHeader *)hList;
	//initialize the list structure

		pList->uiMax_size = uiAmount + sizeof(ListHeader);
	pList->uiTotal_items = 0;
	pList->uiSiz_of_elem = uiSiz_each;
	pList->uiTail = pList->uiHead = sizeof(ListHeader);

	// return the pointer to memory

	return hList;

}


//*****************************************************************************
//
// DeleteQueue
//
// Parameter List : pointer to memory
//
// Return Value	: BOOLEAN
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
BOOLEAN DeleteQueue(HQUEUE hQueue)
{
	if (hQueue == NULL)
	{
		DbgMessage(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "This is not a valid pointer to the queue");
		return FALSE;
	}
	// free the memory assigned to the handle
	MemFree(hQueue);
	return TRUE;
}


//*****************************************************************************
//
// DeleteList
//
// Parameter List : pointer to memory
//
// Return Value	: BOOLEAN
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
BOOLEAN DeleteList(HLIST hList)
{
	if (hList == NULL)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "This is not a valid pointer to the list");
		return FALSE;
	}
	 // free the memory assigned to the list
	MemFree(hList);
	return TRUE;
}


// PeekQueue - gets the first item in queue without
// actually deleting it.
//
// Parameter List : pvoid_queue - pointer to queue
//									container
//									data - data removed from queue
//
// Return Value	pointer to queue with data removed
//							 or NULL if failed
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
static BOOLEAN PeekQueue(HQUEUE hQueue, void* pdata)
{
	QueueHeader *pTemp_cont;
	void *pvoid;
	BYTE *pbyte;

	// cannot check for invalid handle , only 0
	if (hQueue == NULL)
	{
		DbgMessage(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "This is not a valid pointer to the queue");
		return FALSE;
	}
	if (pdata == NULL)
	{
		DbgMessage(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "Memory fo Data to be removed from queue is NULL");
		return FALSE;
	}

	//assign to temporary variables
	pTemp_cont = (QueueHeader *)hQueue;

	// if theres no elements to remove return error
	if (pTemp_cont->uiTotal_items == 0)
	{
		DbgMessage(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "There is nothing in the queue");
		return FALSE;
	}

	//copy the element pointed to by uiHead

	pbyte = (BYTE *)hQueue;
	pbyte += pTemp_cont->uiHead;
	pvoid = (void *)pbyte;
	memmove(pdata, pvoid, pTemp_cont->uiSiz_of_elem);

	return TRUE;
}


//*****************************************************************************
//
// PeekList - gets the specified item in the list without
// actually deleting it.
//
// Parameter List : hList - pointer to list
//									container
//									data - data where list element is stored
//
// Return Value	BOOLEAN
//
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
BOOLEAN PeekList(HLIST hList, void *pdata, UINT32 uiPos)
{
	ListHeader *pTemp_cont;
	void *pvoid;
	UINT32 uiOffsetSrc;
	BYTE *pbyte;

	// cannot check for invalid handle , only 0
	if (hList == NULL)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "This is not a valid pointer to the list");
		return FALSE;
	}
	if (pdata == NULL)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Memory fo Data to be removed from list is NULL");
		return FALSE;
	}

	//assign to temporary variables
	pTemp_cont = (ListHeader *)hList;

	// if theres no elements to peek return error
	if (pTemp_cont->uiTotal_items == 0)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "There is nothing in the list");
		return FALSE;
	}
	if (uiPos >= pTemp_cont->uiTotal_items)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "There is no item at this position");
		return FALSE;
	}

	//copy the element pointed to by uiHead
	uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);
	if (uiOffsetSrc >= pTemp_cont->uiMax_size)
		uiOffsetSrc = sizeof(ListHeader) + (uiOffsetSrc - pTemp_cont->uiMax_size);

	pbyte = (BYTE *)hList;
	pbyte += uiOffsetSrc;
	pvoid = (void *)pbyte;
	memmove(pdata, pvoid, pTemp_cont->uiSiz_of_elem);

	return TRUE;
}


//*****************************************************************************
//
// RemfromQueue
//
// Parameter List : pvoid_queue - pointer to queue
//									container
//									data - data removed from queue
//
// Return Value	pointer to queue with data removed
//							 or NULL if failed
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
BOOLEAN RemfromQueue(HQUEUE hQueue, void *pdata)
{
	QueueHeader *pTemp_cont;
	void *pvoid;
	BYTE *pbyte;

	// cannot check for invalid handle , only 0
	if (hQueue == NULL)
	{
		DbgMessage(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "This is not a valid pointer to the queue");
		return FALSE;
	}
	if (pdata == NULL)
	{
		DbgMessage(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "Memory fo Data to be removed from queue is NULL");
		return FALSE;
	}

	//assign to temporary variables
	pTemp_cont = (QueueHeader *)hQueue;

	// if theres no elements to remove return error
	if (pTemp_cont->uiTotal_items == 0)
	{
		DbgMessage(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "There is nothing in the queue to remove");
		return FALSE;
	}

	//remove the element pointed to by uiHead

	pbyte = (BYTE *)hQueue;
	pbyte += pTemp_cont->uiHead;
	pvoid = (void *)pbyte;
	memmove(pdata, pvoid, pTemp_cont->uiSiz_of_elem);
		pTemp_cont->uiTotal_items--;
	pTemp_cont->uiHead += pTemp_cont->uiSiz_of_elem;

	// if after removing an element head = tail then set them both
	// to the beginning of the container as it is empty

	if (pTemp_cont->uiHead == pTemp_cont->uiTail)
		pTemp_cont->uiHead = pTemp_cont->uiTail = sizeof(QueueHeader);

	// if only the head is at the end of the container then make it point
	// to the beginning of the container

	if (pTemp_cont->uiHead == pTemp_cont->uiMax_size)
		pTemp_cont->uiHead = sizeof(QueueHeader);

	return TRUE;
}


//*****************************************************************************
//
// AddtoQueue
//
// Parameter List : pvoid_queue - pointer to queue
//									container
//									pdata - pointer to data to add to queue
//
// Return Value	pointer to queue with data added
//							 else	false
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
HQUEUE AddtoQueue(HQUEUE hQueue, void *pdata)
{
	QueueHeader *pTemp_cont;
	UINT32 uiMax_size;
	UINT32 uiSize_of_each;
	UINT32 uiTotal;
	UINT32 uiNew_size;
	UINT32 uiHead;
	UINT32 uiTail;
	void *pvoid;
	BYTE *pbyte;
	BYTE *pmaxsize;
	BYTE *presize;
	BOOLEAN fresize;

	// check for invalid handle = 0
	if (hQueue == NULL)
	{
		DbgMessage(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "This is not a valid pointer to the queue");
		return NULL;
	}

	// check for data = NULL
	if (pdata == NULL)
	{
		DbgMessage(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "Data to be added onto queue is NULL");
		return NULL;
	}

	// assign some temporary variables
	fresize = FALSE;
	pTemp_cont = (QueueHeader *)hQueue;
		uiTotal = pTemp_cont->uiTotal_items;
		uiSize_of_each = pTemp_cont->uiSiz_of_elem;
	uiMax_size = pTemp_cont->uiMax_size;
	uiHead = pTemp_cont->uiHead;
	uiTail = pTemp_cont->uiTail;
	if ((uiTail + uiSize_of_each) > uiMax_size)
	{
		uiTail = pTemp_cont->uiTail = sizeof(QueueHeader);
		fresize = TRUE;
	}
	if ((uiHead == uiTail) && ((uiHead >= (sizeof(QueueHeader) + uiSize_of_each)) || (fresize == TRUE)))
	{
		uiNew_size = uiMax_size + (uiMax_size - sizeof(QueueHeader));
		pTemp_cont->uiMax_size = uiNew_size;
		if ((hQueue = MemRealloc(hQueue, uiNew_size)) == NULL)
		{
		 DbgMessage(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "Could not resize queue container memory");
			 return NULL;
		}
		// copy memory from beginning of container to end of container
	// so that all the data is in one continuous block

		pTemp_cont = (QueueHeader *)hQueue;
		presize = (BYTE *)hQueue;
		pmaxsize = (BYTE *)hQueue;
		presize += sizeof(QueueHeader);
		pmaxsize += uiMax_size;
		if (uiHead > sizeof(QueueHeader))
			memmove(pmaxsize, presize, uiHead-sizeof(QueueHeader));
		pTemp_cont->uiTail = uiMax_size + (uiHead-sizeof(QueueHeader));
	}
	pbyte = (BYTE *)hQueue;
	pbyte += pTemp_cont->uiTail;
	pvoid = (void *)pbyte;
	memmove(pvoid, pdata, uiSize_of_each);
		pTemp_cont->uiTotal_items++;
	pTemp_cont->uiTail += uiSize_of_each;
	return hQueue;
}


//*****************************************************************************
//
// do_copy
//
// Parameter List : pointer to mem, source offset, dest offset, size
//
// Return Value	BOOLEAN
//
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
static BOOLEAN do_copy(void* pmem_void, UINT32 uiSourceOfst, UINT32 uiDestOfst, UINT32 uiSize)
{
	BYTE *pOffsetSrc;
	BYTE *pOffsetDst;
	void *pvoid_src;
	void *pvoid_dest;

	if ((uiSourceOfst < 0) || (uiDestOfst < 0) || (uiSize < 0))
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Invalid parameters passed to do_copy");
		return FALSE;
	}

		if (pmem_void == NULL)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Invalid pointer passed to do_copy");
		return FALSE;
	}
		pOffsetSrc = (BYTE *)pmem_void;
	pOffsetSrc += uiSourceOfst;
	pOffsetDst = (BYTE *)pmem_void;
	pOffsetDst += uiDestOfst;
	pvoid_src = (void *)pOffsetSrc;
	pvoid_dest = (void *)pOffsetDst;
	memmove(pvoid_dest, pvoid_src, uiSize);
	return TRUE;
}


//*****************************************************************************
//
// do_copy_data
//
// Parameter List : pointer to mem, pointer to data, source offset, size
//
// Return Value	BOOLEAN
//
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
static BOOLEAN do_copy_data(void* pmem_void, void* data, UINT32 uiSrcOfst, UINT32 uiSize)
{
	BYTE *pOffsetSrc;
	void *pvoid_src;

	if ((uiSrcOfst < 0) || (uiSize < 0))
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Invalid parameters passed to do_copy_data");
		return FALSE;
	}

		if (pmem_void == NULL)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Invalid pointer passed to do_copy_data");
		return FALSE;
	}
		pOffsetSrc = (BYTE *)pmem_void;
	pOffsetSrc += uiSrcOfst;
	pvoid_src = (void *)pOffsetSrc;
	memmove(data, pvoid_src, uiSize);
	return TRUE;
}


//*****************************************************************************
//
// QueueSize
//
// Parameter List : pointer to queue
//
// Return Value	UINT32 queue size
//
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
UINT32 QueueSize(HQUEUE hQueue)
{
	QueueHeader *pTemp_cont;
	if (hQueue == NULL)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Queue pointer is NULL");
		return 0;
	}
	pTemp_cont = (QueueHeader *)hQueue;
	return pTemp_cont->uiTotal_items;
}


//*****************************************************************************
//
// ListSize
//
// Parameter List : pointer to queue
//
// Return Value	UINT32 list size
//
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
UINT32 ListSize(HLIST hList)
{
	ListHeader *pTemp_cont;
	if (hList == NULL)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "List pointer is NULL");
		return 0;
	}
	pTemp_cont = (ListHeader *)hList;
	return pTemp_cont->uiTotal_items;
}


//*****************************************************************************
//
// AddtoList
//
// Parameter List : HCONTAINER - handle to list
//									container
//									data - data to add to queue
//									position - position after which data is to added
//
// Return Value	BOOLEAN true if push ok
//							 else	false
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
HLIST AddtoList(HLIST hList, void *pdata, UINT32 uiPos)
{
	ListHeader *pTemp_cont;
	UINT32 uiMax_size;
	UINT32 uiSize_of_each;
	UINT32 uiTotal;
	UINT32 uiNew_size;
	UINT32 uiHead;
	UINT32 uiTail;
	void *pvoid;
	BYTE *pbyte;
	UINT32 uiOffsetSrc;
	UINT32 uiOffsetDst;
	UINT32 uiFinalLoc = 0;
	BOOLEAN fTail_check=FALSE;

	// check for invalid handle = 0
	if (hList == NULL)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "This is not a valid handle to the list");
		return NULL;
	}

	// check for data = NULL
	if (pdata == NULL)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Data to be pushed onto list is NULL");
		return NULL;
	}
	// check for a 0 or negative position passed in
	if (uiPos < 0)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Data to be pushed onto list is NULL");
		return NULL;
	}

	// assign some temporary variables

	pTemp_cont = (ListHeader *)hList;
	if (uiPos > pTemp_cont->uiTotal_items)
	{
			DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "There are not enough elements in the list");
		return NULL;
	}
		uiTotal = pTemp_cont->uiTotal_items;
		uiSize_of_each = pTemp_cont->uiSiz_of_elem;
	uiMax_size = pTemp_cont->uiMax_size;
	uiHead = pTemp_cont->uiHead;
	uiTail = pTemp_cont->uiTail;
	uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);
	if (uiOffsetSrc >= uiMax_size)
		uiOffsetSrc = sizeof(ListHeader) + (uiOffsetSrc - uiMax_size);
	if (uiTail == uiOffsetSrc)
		fTail_check = TRUE;
	// copy appropriate blocks
	if (((uiTail + uiSize_of_each) <= uiMax_size) &&
		((uiTail > uiHead) || ((uiTail == uiHead) && (uiHead == sizeof(ListHeader)))))
	{
		uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);
			uiOffsetDst = uiOffsetSrc + pTemp_cont->uiSiz_of_elem;
		if (fTail_check == FALSE)
		{
			if (do_copy(hList, uiOffsetSrc, uiOffsetDst, uiTail-uiOffsetSrc) == FALSE)
			{
				 DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
			 return NULL;
			}
		}
		if (fTail_check == FALSE)
				pTemp_cont->uiTail += uiSize_of_each;
		uiFinalLoc = uiOffsetSrc;
	}


		if ((((uiTail + uiSize_of_each) <= uiMax_size) && (uiTail < uiHead))
	|| (((uiTail + uiSize_of_each) > uiMax_size) && (uiHead >= (sizeof(ListHeader) + uiSize_of_each))))
	{
	uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);

		if (uiOffsetSrc >= uiMax_size)
		{
			uiOffsetSrc = sizeof(ListHeader) + (uiOffsetSrc - uiMax_size);
			uiOffsetDst = uiOffsetSrc + uiSize_of_each;
			if (do_copy(hList, uiOffsetDst, uiOffsetSrc, uiTail-uiOffsetSrc) == FALSE)
				{
					 DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
					 return NULL;
				}
			uiFinalLoc = uiOffsetSrc;
		} else
		{
			uiOffsetSrc = sizeof(ListHeader);
			uiOffsetDst = uiOffsetSrc + uiSize_of_each;
			if (do_copy(hList, uiOffsetSrc, uiOffsetDst, uiTail-uiOffsetSrc) == FALSE)
				{
					 DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
					 return NULL;
				}

			uiOffsetSrc = uiMax_size - uiSize_of_each;
			uiOffsetDst = sizeof(ListHeader);
			if (do_copy(hList, uiOffsetSrc, uiOffsetDst, uiSize_of_each) == FALSE)
				{
					 DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
					 return NULL;
				}
				uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);
			uiOffsetDst = uiOffsetSrc + uiSize_of_each;
				if (do_copy(hList, uiOffsetSrc, uiOffsetDst, (uiMax_size-uiSize_of_each) - uiOffsetSrc) == FALSE)
				{
					 DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
					 return NULL;
				}
		}
				 pTemp_cont->uiTail += uiSize_of_each;
			 uiFinalLoc = uiOffsetSrc;
	}// end if



	if ((((uiTail + uiSize_of_each) <= uiMax_size) && (uiTail == uiHead) && (uiHead >= (sizeof(ListHeader) + uiSize_of_each)))
		 || (((uiTail + uiSize_of_each) > uiMax_size) && (uiHead == sizeof(ListHeader))))
	{
		// need to resize the container
		uiNew_size = uiMax_size + (uiMax_size - sizeof(ListHeader));
		pTemp_cont->uiMax_size = uiNew_size;
		if ((hList = MemRealloc(hList, uiNew_size)) == NULL)
		{
				DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not resize list container memory");
				return NULL;
			}
		pTemp_cont = (ListHeader *)hList;
		if (do_copy(hList, sizeof(ListHeader), uiMax_size, uiHead - sizeof(ListHeader)) == FALSE)
		{
				DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not copy list container memory");
				return NULL;
			}
		pTemp_cont->uiTail = uiMax_size + (uiHead-sizeof(ListHeader));

		// now make place for the actual element

				uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);
				uiOffsetDst = uiOffsetSrc + pTemp_cont->uiSiz_of_elem;
			if (do_copy(hList, uiOffsetSrc, uiOffsetDst, uiTail-uiOffsetSrc) == FALSE)
			{
				 DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
			 return NULL;
			}
				pTemp_cont->uiTail += uiSize_of_each;
			uiFinalLoc = uiOffsetSrc;
		}


		// finally insert data at position uiFinalLoc

	pbyte = (BYTE *)hList;
	if (uiFinalLoc == 0)
	{
			DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "This should never happen! report this problem!");
		return NULL;
	}
	pbyte += uiFinalLoc;
	pvoid = (void *)pbyte;

	memmove(pvoid, pdata, pTemp_cont->uiSiz_of_elem);
		pTemp_cont->uiTotal_items++;
	if (fTail_check == TRUE)
		pTemp_cont->uiTail += pTemp_cont->uiSiz_of_elem;
	return hList;
}


//*****************************************************************************
//
// RemfromList
//
// Parameter List : HLIST - handle to list
//									container
//									data - data to remove from list
//									position - position after which data is to added
//
// Return Value	BOOLEAN true if push ok
//							 else	false
//
// Modification History :
// Nov 26th 1996 -> modified for use by Wizardry
//
//*****************************************************************************
BOOLEAN RemfromList(HLIST hList, void *pdata, UINT32 uiPos)
{
	ListHeader *pTemp_cont;
	UINT32 uiMax_size;
	UINT32 uiSize_of_each;
	UINT32 uiTotal;
	UINT32 uiHead;
	UINT32 uiTail;
	UINT32 uiOffsetSrc;
	UINT32 uiOffsetDst;
	UINT32 uiFinalLoc = 0;

	// check for invalid handle = 0
	if (hList == NULL)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "This is not a valid handle to the list");
		return FALSE;
	}

	// check for data = NULL
	if (pdata == NULL)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Data to be pushed onto list is NULL");
		return FALSE;
	}
	// check for a 0 or negative position passed in
	if (uiPos < 0)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Data to be pushed onto list is NULL");
		return FALSE;
	}

	// assign some temporary variables
	pTemp_cont = (ListHeader *)hList;

	if (uiPos >= pTemp_cont->uiTotal_items)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Cannot delete at the specified position");
		return FALSE;
	}
	if (pTemp_cont->uiTotal_items == 0)
	{
		DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "There are no elements in the list to remove");
		return FALSE;
	}

	uiTotal				= pTemp_cont->uiTotal_items;
	uiSize_of_each = pTemp_cont->uiSiz_of_elem;
	uiMax_size		 = pTemp_cont->uiMax_size;
	uiHead				 = pTemp_cont->uiHead;
	uiTail				 = pTemp_cont->uiTail;

	// copy appropriate blocks
	if ((uiTail > uiHead) || ((uiTail == uiHead) && (uiHead == sizeof(ListHeader))))
	{
		uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);
		uiOffsetDst = uiOffsetSrc + pTemp_cont->uiSiz_of_elem;
		if (do_copy_data(hList, pdata, uiOffsetSrc, uiSize_of_each) == FALSE)
		{
			DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not copy the data from list");
			return FALSE;
		}
		if (do_copy(hList, uiOffsetDst, uiOffsetSrc, uiTail-uiOffsetSrc) == FALSE)
		{
			DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not remove the data the list");
			return FALSE;
		}
		pTemp_cont->uiTail -= uiSize_of_each;
		pTemp_cont->uiTotal_items--;
	}


	if ((uiTail < uiHead) || ((uiTail == uiHead) && (uiHead <= (sizeof(ListHeader)+uiSize_of_each))))
	{
		uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);

		if (uiOffsetSrc >= uiMax_size)
		{
			uiOffsetSrc = sizeof(ListHeader) + (uiOffsetSrc - uiMax_size);
			uiOffsetDst = uiOffsetSrc + uiSize_of_each;
			if (do_copy_data(hList, pdata, uiOffsetSrc, uiSize_of_each) == FALSE)
			{
				DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not copy the data from list");
				return FALSE;
			}
			if (do_copy(hList, uiOffsetSrc, uiOffsetDst, uiTail-uiOffsetSrc) == FALSE)
			{
				DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
				return FALSE;
			}
			uiFinalLoc = uiOffsetSrc;
		}
		else
		{
			uiOffsetSrc = sizeof(ListHeader);
			uiOffsetDst = uiOffsetSrc + uiSize_of_each;
			if (do_copy(hList, uiOffsetSrc, uiOffsetDst, uiTail-uiOffsetSrc) == FALSE)
			{
				DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
				return FALSE;
			}

			uiOffsetSrc = uiMax_size - uiSize_of_each;
			uiOffsetDst = sizeof(ListHeader);
			if (do_copy(hList, uiOffsetSrc, uiOffsetDst, uiSize_of_each) == FALSE)
			{
				DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
				return FALSE;
			}
			uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);
			uiOffsetDst = uiOffsetSrc + uiSize_of_each;
			if (do_copy_data(hList, pdata, uiOffsetSrc, uiSize_of_each) == FALSE)
			{
				DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not copy the data from list");
				return FALSE;
			}
			if (do_copy(hList, uiOffsetSrc, uiOffsetDst, (uiMax_size-uiSize_of_each) - uiOffsetSrc) == FALSE)
			{
				DbgMessage(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
				return FALSE;
			}
		}
		pTemp_cont->uiTail -= uiSize_of_each;
		pTemp_cont->uiTotal_items--;
	} // end if

	return TRUE;
}
