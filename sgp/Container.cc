//*****************************************************************************
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


typedef struct ListHeader
{
	UINT32 uiTotal_items;
	UINT32 uiSiz_of_elem;
	UINT32 uiMax_size;
	UINT32 uiHead;
	UINT32 uiTail;
} ListHeader;


typedef struct QueueHeader
{
	UINT32 uiTotal_items;
	UINT32 uiSiz_of_elem;
	UINT32 uiMax_size;
	UINT32 uiHead;
	UINT32 uiTail;
} QueueHeader;


// Parameter List : num_items - estimated number of items in queue
//									siz_each - size of each item
// Return Value	NULL if unsuccesful
//							 pointer to allocated memory
HQUEUE CreateQueue(UINT32 uiNum_items, UINT32 uiSiz_each)
try
{
	if (uiNum_items == 0 || uiSiz_each == 0)
	{
		DebugMsg(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "Requested queue items and size have to be >0");
		return NULL;
	}

	UINT32 uiAmount = uiNum_items * uiSiz_each;

	const HQUEUE hQueue = MALLOCE(QueueHeader, uiAmount);

	QueueHeader* pQueue = hQueue;
	pQueue->uiMax_size = uiAmount + sizeof(QueueHeader);
	pQueue->uiTotal_items = 0;
	pQueue->uiSiz_of_elem = uiSiz_each;
	pQueue->uiTail = pQueue->uiHead = sizeof(QueueHeader);

	return hQueue;
}
catch (...) { return 0; }


// Parameter List : num_items - estimated number of items in ordered list
//									siz_each - size of each item
// Return Value	NULL if unsuccesful
//							 pointer to allocated memory
HLIST CreateList(UINT32 uiNum_items, UINT32 uiSiz_each)
try
{
	if (uiNum_items == 0 || uiSiz_each == 0)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Requested queue items and size have to be >0");
		return 0;
	}

	UINT32 uiAmount = uiNum_items * uiSiz_each;

	const HLIST hList = MALLOCE(ListHeader, uiAmount);

	ListHeader* pList = hList;
	pList->uiMax_size = uiAmount + sizeof(ListHeader);
	pList->uiTotal_items = 0;
	pList->uiSiz_of_elem = uiSiz_each;
	pList->uiTail = pList->uiHead = sizeof(ListHeader);

	return hList;
}
catch (...) { return 0; }


BOOLEAN DeleteQueue(HQUEUE hQueue)
{
	if (hQueue == NULL)
	{
		DebugMsg(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "This is not a valid pointer to the queue");
		return FALSE;
	}
	MemFree(hQueue);
	return TRUE;
}


BOOLEAN DeleteList(HLIST hList)
{
	if (hList == NULL)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "This is not a valid pointer to the list");
		return FALSE;
	}
	MemFree(hList);
	return TRUE;
}


// PeekList - gets the specified item in the list without actually deleting it.
//
// Parameter List : hList - pointer to list container
//									data - data where list element is stored
BOOLEAN PeekList(HLIST hList, void *pdata, UINT32 uiPos)
{
	// cannot check for invalid handle , only 0
	if (hList == NULL)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "This is not a valid pointer to the list");
		return FALSE;
	}
	if (pdata == NULL)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Memory fo Data to be removed from list is NULL");
		return FALSE;
	}

	ListHeader* pTemp_cont = hList;

	// if theres no elements to peek return error
	if (pTemp_cont->uiTotal_items == 0)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "There is nothing in the list");
		return FALSE;
	}
	if (uiPos >= pTemp_cont->uiTotal_items)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "There is no item at this position");
		return FALSE;
	}

	//copy the element pointed to by uiHead
	UINT32 uiOffsetSrc = pTemp_cont->uiHead + uiPos * pTemp_cont->uiSiz_of_elem;
	if (uiOffsetSrc >= pTemp_cont->uiMax_size)
		uiOffsetSrc = sizeof(ListHeader) + (uiOffsetSrc - pTemp_cont->uiMax_size);

	BYTE* pbyte = (BYTE*)hList;
	pbyte += uiOffsetSrc;
	void* pvoid = pbyte;
	memmove(pdata, pvoid, pTemp_cont->uiSiz_of_elem);

	return TRUE;
}


// Parameter List : pvoid_queue - pointer to queue container
//									data - data removed from queue
BOOLEAN RemfromQueue(HQUEUE hQueue, void *pdata)
{
	// cannot check for invalid handle , only 0
	if (hQueue == NULL)
	{
		DebugMsg(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "This is not a valid pointer to the queue");
		return FALSE;
	}
	if (pdata == NULL)
	{
		DebugMsg(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "Memory fo Data to be removed from queue is NULL");
		return FALSE;
	}

	QueueHeader* pTemp_cont = hQueue;

	// if theres no elements to remove return error
	if (pTemp_cont->uiTotal_items == 0)
	{
		DebugMsg(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "There is nothing in the queue to remove");
		return FALSE;
	}

	//remove the element pointed to by uiHead

	BYTE* pbyte = (BYTE*)hQueue + pTemp_cont->uiHead;
	memmove(pdata, pbyte, pTemp_cont->uiSiz_of_elem);
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


// Parameter List : pvoid_queue - pointer to queue container
//									pdata - pointer to data to add to queue
//
// Return Value	pointer to queue with data added else	NULL
HQUEUE AddtoQueue(HQUEUE hQueue, void const* const pdata)
{
	// check for invalid handle = 0
	if (hQueue == NULL)
	{
		DebugMsg(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "This is not a valid pointer to the queue");
		return NULL;
	}

	// check for data = NULL
	if (pdata == NULL)
	{
		DebugMsg(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "Data to be added onto queue is NULL");
		return NULL;
	}

	// assign some temporary variables
	BOOLEAN fresize = FALSE;
	QueueHeader* pTemp_cont = hQueue;
	UINT32 uiSize_of_each = pTemp_cont->uiSiz_of_elem;
	UINT32 uiMax_size = pTemp_cont->uiMax_size;
	UINT32 uiHead = pTemp_cont->uiHead;
	UINT32 uiTail = pTemp_cont->uiTail;
	if (uiTail + uiSize_of_each > uiMax_size)
	{
		uiTail = pTemp_cont->uiTail = sizeof(QueueHeader);
		fresize = TRUE;
	}
	if (uiHead == uiTail && (uiHead >= sizeof(QueueHeader) + uiSize_of_each || fresize))
	{
		UINT32 uiNew_size = uiMax_size + (uiMax_size - sizeof(QueueHeader));
		pTemp_cont->uiMax_size = uiNew_size;
		hQueue = (HQUEUE)MemRealloc(hQueue, uiNew_size);
		if (hQueue == NULL)
		{
			DebugMsg(TOPIC_QUEUE_CONTAINERS, DBG_LEVEL_0, "Could not resize queue container memory");
			return NULL;
		}
		// copy memory from beginning of container to end of container
		// so that all the data is in one continuous block

		pTemp_cont = hQueue;
		BYTE* presize = (BYTE*)hQueue;
		BYTE* pmaxsize = (BYTE*)hQueue;
		presize += sizeof(QueueHeader);
		pmaxsize += uiMax_size;
		if (uiHead > sizeof(QueueHeader))
			memmove(pmaxsize, presize, uiHead-sizeof(QueueHeader));
		pTemp_cont->uiTail = uiMax_size + (uiHead-sizeof(QueueHeader));
	}
	BYTE* pbyte = (BYTE*)hQueue + pTemp_cont->uiTail;
	memmove(pbyte, pdata, uiSize_of_each);
	pTemp_cont->uiTotal_items++;
	pTemp_cont->uiTail += uiSize_of_each;
	return hQueue;
}


static BOOLEAN do_copy(void* pmem_void, UINT32 uiSourceOfst, UINT32 uiDestOfst, UINT32 uiSize)
{
	if (pmem_void == NULL)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Invalid pointer passed to do_copy");
		return FALSE;
	}
	BYTE* pOffsetSrc = (BYTE*)pmem_void + uiSourceOfst;
	BYTE* pOffsetDst = (BYTE*)pmem_void + uiDestOfst;
	memmove(pOffsetDst, pOffsetSrc, uiSize);
	return TRUE;
}


static BOOLEAN do_copy_data(void* pmem_void, void* data, UINT32 uiSrcOfst, UINT32 uiSize)
{
	if (pmem_void == NULL)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Invalid pointer passed to do_copy_data");
		return FALSE;
	}
	BYTE* pOffsetSrc = (BYTE*)pmem_void + uiSrcOfst;
	memmove(data, pOffsetSrc, uiSize);
	return TRUE;
}


// Parameter List : pointer to queue
//
// Return Value	UINT32 queue size
UINT32 QueueSize(HQUEUE hQueue)
{
	if (hQueue == NULL)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Queue pointer is NULL");
		return 0;
	}
	return hQueue->uiTotal_items;
}


// Parameter List : pointer to queue
//
// Return Value	UINT32 list size
UINT32 ListSize(HLIST hList)
{
	if (hList == NULL)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "List pointer is NULL");
		return 0;
	}
	return hList->uiTotal_items;
}


// Parameter List : HCONTAINER - handle to list container
//									data - data to add to queue
//									position - position after which data is to added
HLIST AddtoList(HLIST hList, void const* pdata, UINT32 const uiPos)
{
	UINT32 uiOffsetDst;
	UINT32 uiFinalLoc = 0;
	BOOLEAN fTail_check = FALSE;

	// check for invalid handle = 0
	if (hList == NULL)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "This is not a valid handle to the list");
		return NULL;
	}

	// check for data = NULL
	if (pdata == NULL)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Data to be pushed onto list is NULL");
		return NULL;
	}

	ListHeader* pTemp_cont = hList;
	if (uiPos > pTemp_cont->uiTotal_items)
	{
			DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "There are not enough elements in the list");
		return NULL;
	}
	UINT32 uiSize_of_each = pTemp_cont->uiSiz_of_elem;
	UINT32 uiMax_size = pTemp_cont->uiMax_size;
	UINT32 uiHead = pTemp_cont->uiHead;
	UINT32 uiTail = pTemp_cont->uiTail;
	UINT32 uiOffsetSrc = pTemp_cont->uiHead + uiPos * pTemp_cont->uiSiz_of_elem;
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
		if (!fTail_check)
		{
			if (!do_copy(hList, uiOffsetSrc, uiOffsetDst, uiTail - uiOffsetSrc))
			{
				DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
				return NULL;
			}
		}
		if (!fTail_check)
			pTemp_cont->uiTail += uiSize_of_each;
		uiFinalLoc = uiOffsetSrc;
	}


	if ((((uiTail + uiSize_of_each) <= uiMax_size) && (uiTail < uiHead)) ||
			(((uiTail + uiSize_of_each) > uiMax_size) && (uiHead >= (sizeof(ListHeader) + uiSize_of_each))))
	{
		uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);

		if (uiOffsetSrc >= uiMax_size)
		{
			uiOffsetSrc = sizeof(ListHeader) + (uiOffsetSrc - uiMax_size);
			uiOffsetDst = uiOffsetSrc + uiSize_of_each;
			if (!do_copy(hList, uiOffsetDst, uiOffsetSrc, uiTail - uiOffsetSrc))
			{
				 DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
				 return NULL;
			}
			uiFinalLoc = uiOffsetSrc;
		}
		else
		{
			uiOffsetSrc = sizeof(ListHeader);
			uiOffsetDst = uiOffsetSrc + uiSize_of_each;
			if (!do_copy(hList, uiOffsetSrc, uiOffsetDst, uiTail - uiOffsetSrc))
			{
				 DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
				 return NULL;
			}

			uiOffsetSrc = uiMax_size - uiSize_of_each;
			uiOffsetDst = sizeof(ListHeader);
			if (!do_copy(hList, uiOffsetSrc, uiOffsetDst, uiSize_of_each))
			{
				 DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
				 return NULL;
			}
			uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);
			uiOffsetDst = uiOffsetSrc + uiSize_of_each;
			if (!do_copy(hList, uiOffsetSrc, uiOffsetDst, uiMax_size - uiSize_of_each - uiOffsetSrc))
			{
				DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
				return NULL;
			}
		}
		pTemp_cont->uiTail += uiSize_of_each;
		uiFinalLoc = uiOffsetSrc;
	}


	if ((((uiTail + uiSize_of_each) <= uiMax_size) && (uiTail == uiHead) && (uiHead >= (sizeof(ListHeader) + uiSize_of_each))) ||
			(((uiTail + uiSize_of_each) > uiMax_size) && (uiHead == sizeof(ListHeader))))
	{
		// need to resize the container
		UINT32 uiNew_size = uiMax_size + (uiMax_size - sizeof(ListHeader));
		pTemp_cont->uiMax_size = uiNew_size;
		hList = (HLIST)MemRealloc(hList, uiNew_size);
		if (hList == NULL)
		{
			DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not resize list container memory");
			return NULL;
		}
		pTemp_cont = hList;
		if (!do_copy(hList, sizeof(ListHeader), uiMax_size, uiHead - sizeof(ListHeader)))
		{
			DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not copy list container memory");
			return NULL;
		}
		pTemp_cont->uiTail = uiMax_size + (uiHead-sizeof(ListHeader));

		// now make place for the actual element
		uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);
		uiOffsetDst = uiOffsetSrc + pTemp_cont->uiSiz_of_elem;
		if (!do_copy(hList, uiOffsetSrc, uiOffsetDst, uiTail - uiOffsetSrc))
		{
			DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
			return NULL;
		}
		pTemp_cont->uiTail += uiSize_of_each;
		uiFinalLoc = uiOffsetSrc;
	}


	// finally insert data at position uiFinalLoc
	if (uiFinalLoc == 0)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "This should never happen! report this problem!");
		return NULL;
	}

	BYTE* pbyte = (BYTE*)hList + uiFinalLoc;
	memmove(pbyte, pdata, pTemp_cont->uiSiz_of_elem);
	pTemp_cont->uiTotal_items++;
	if (fTail_check) pTemp_cont->uiTail += pTemp_cont->uiSiz_of_elem;
	return hList;
}


// Parameter List : HLIST - handle to list container
//									data - data to remove from list
//									position - position after which data is to added
BOOLEAN RemfromList(HLIST hList, void *pdata, UINT32 uiPos)
{
	// check for invalid handle = 0
	if (hList == NULL)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "This is not a valid handle to the list");
		return FALSE;
	}

	// check for data = NULL
	if (pdata == NULL)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Data to be pushed onto list is NULL");
		return FALSE;
	}

	ListHeader* pTemp_cont = hList;

	if (uiPos >= pTemp_cont->uiTotal_items)
	{
		DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Cannot delete at the specified position");
		return FALSE;
	}

	UINT32 uiSize_of_each = pTemp_cont->uiSiz_of_elem;
	UINT32 uiMax_size     = pTemp_cont->uiMax_size;
	UINT32 uiHead         = pTemp_cont->uiHead;
	UINT32 uiTail         = pTemp_cont->uiTail;

	// copy appropriate blocks
	if ((uiTail > uiHead) || ((uiTail == uiHead) && (uiHead == sizeof(ListHeader))))
	{
		UINT32 uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);
		UINT32 uiOffsetDst = uiOffsetSrc + pTemp_cont->uiSiz_of_elem;
		if (!do_copy_data(hList, pdata, uiOffsetSrc, uiSize_of_each))
		{
			DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not copy the data from list");
			return FALSE;
		}
		if (!do_copy(hList, uiOffsetDst, uiOffsetSrc, uiTail - uiOffsetSrc))
		{
			DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not remove the data the list");
			return FALSE;
		}
		pTemp_cont->uiTail -= uiSize_of_each;
		pTemp_cont->uiTotal_items--;
	}


	if ((uiTail < uiHead) || ((uiTail == uiHead) && (uiHead <= (sizeof(ListHeader)+uiSize_of_each))))
	{
		UINT32 uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);

		if (uiOffsetSrc >= uiMax_size)
		{
			uiOffsetSrc = sizeof(ListHeader) + (uiOffsetSrc - uiMax_size);
			UINT32 uiOffsetDst = uiOffsetSrc + uiSize_of_each;
			if (!do_copy_data(hList, pdata, uiOffsetSrc, uiSize_of_each))
			{
				DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not copy the data from list");
				return FALSE;
			}
			if (!do_copy(hList, uiOffsetSrc, uiOffsetDst, uiTail - uiOffsetSrc))
			{
				DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
				return FALSE;
			}
		}
		else
		{
			UINT32 uiOffsetSrc = sizeof(ListHeader);
			UINT32 uiOffsetDst = uiOffsetSrc + uiSize_of_each;
			if (!do_copy(hList, uiOffsetSrc, uiOffsetDst, uiTail - uiOffsetSrc))
			{
				DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
				return FALSE;
			}

			uiOffsetSrc = uiMax_size - uiSize_of_each;
			uiOffsetDst = sizeof(ListHeader);
			if (!do_copy(hList, uiOffsetSrc, uiOffsetDst, uiSize_of_each))
			{
				DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
				return FALSE;
			}
			uiOffsetSrc = pTemp_cont->uiHead + (uiPos*pTemp_cont->uiSiz_of_elem);
			uiOffsetDst = uiOffsetSrc + uiSize_of_each;
			if (!do_copy_data(hList, pdata, uiOffsetSrc, uiSize_of_each))
			{
				DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not copy the data from list");
				return FALSE;
			}
			if (!do_copy(hList, uiOffsetSrc, uiOffsetDst, uiMax_size - uiSize_of_each - uiOffsetSrc))
			{
				DebugMsg(TOPIC_LIST_CONTAINERS, DBG_LEVEL_0, "Could not store the data in list");
				return FALSE;
			}
		}
		pTemp_cont->uiTail -= uiSize_of_each;
		pTemp_cont->uiTotal_items--;
	}

	return TRUE;
}
