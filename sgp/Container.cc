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
#include <stdexcept>

#include "Container.h"
#include "Debug.h"
#include "MemMan.h"
#include "Types.h"


struct ListHeader
{
	UINT32 uiTotal_items;
	UINT32 uiSiz_of_elem;
	UINT32 uiMax_size;
	UINT32 uiHead;
	UINT32 uiTail;
};


struct QueueHeader
{
	UINT32 uiTotal_items;
	UINT32 uiSiz_of_elem;
	UINT32 uiMax_size;
	UINT32 uiHead;
	UINT32 uiTail;
};


// Parameter List : num_items - estimated number of items in queue
//									siz_each - size of each item
// Return Value	NULL if unsuccesful
//							 pointer to allocated memory
HQUEUE CreateQueue(UINT32 const uiNum_items, UINT32 const uiSiz_each)
{
	if (uiNum_items == 0 || uiSiz_each == 0)
	{
		throw std::logic_error("Requested queue items and size have to be >0");
	}

	UINT32 const uiAmount = uiNum_items * uiSiz_each;
	HQUEUE const q        = MALLOCE(QueueHeader, uiAmount);
	q->uiMax_size    = uiAmount + sizeof(QueueHeader);
	q->uiTotal_items = 0;
	q->uiSiz_of_elem = uiSiz_each;
	q->uiTail        = sizeof(QueueHeader);
	q->uiHead        = sizeof(QueueHeader);
	return q;
}


// Parameter List : num_items - estimated number of items in ordered list
//									siz_each - size of each item
// Return Value	NULL if unsuccesful
//							 pointer to allocated memory
HLIST CreateList(UINT32 const uiNum_items, UINT32 const uiSiz_each)
{
	if (uiNum_items == 0 || uiSiz_each == 0)
	{
		throw std::logic_error("Requested queue items and size have to be >0");
	}

	UINT32 const uiAmount = uiNum_items * uiSiz_each;
	HLIST  const l        = MALLOCE(ListHeader, uiAmount);
	l->uiMax_size    = uiAmount + sizeof(ListHeader);
	l->uiTotal_items = 0;
	l->uiSiz_of_elem = uiSiz_each;
	l->uiTail        = sizeof(ListHeader);
	l->uiHead    = sizeof(ListHeader);
	return l;
}


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
void PeekList(HLIST const l, void* const data, UINT32 const pos)
{
	if (pos >= l->uiTotal_items)
	{
		throw std::logic_error("Tried to peek at non-existent element in list");
	}

	UINT32 uiOffsetSrc = l->uiHead + pos * l->uiSiz_of_elem;
	if (uiOffsetSrc >= l->uiMax_size)
		uiOffsetSrc = sizeof(ListHeader) + (uiOffsetSrc - l->uiMax_size);

	BYTE const* const pbyte = (BYTE*)l + uiOffsetSrc;
	memmove(data, pbyte, l->uiSiz_of_elem);
}


// Parameter List : pvoid_queue - pointer to queue container
//									data - data removed from queue
void RemfromQueue(HQUEUE const q, void* const data)
{
	if (!q)                    throw std::logic_error("This is not a valid pointer to the queue");
	if (!data)                 throw std::logic_error("Memory fo Data to be removed from queue is NULL");
	if (q->uiTotal_items == 0) throw std::logic_error("There is nothing in the queue to remove");

	//remove the element pointed to by uiHead

	BYTE* const pbyte = (BYTE*)q + q->uiHead;
	memmove(data, pbyte, q->uiSiz_of_elem);
	q->uiTotal_items--;
	q->uiHead += q->uiSiz_of_elem;

	// if after removing an element head = tail then set them both
	// to the beginning of the container as it is empty

	if (q->uiHead == q->uiTail) q->uiHead = q->uiTail = sizeof(QueueHeader);

	// if only the head is at the end of the container then make it point
	// to the beginning of the container

	if (q->uiHead == q->uiMax_size) q->uiHead = sizeof(QueueHeader);
}


// Parameter List : pvoid_queue - pointer to queue container
//									pdata - pointer to data to add to queue
//
// Return Value	pointer to queue with data added
HQUEUE AddtoQueue(HQUEUE q, void const* const pdata)
{
	if (!q)     throw std::logic_error("This is not a valid pointer to the queue");
	if (!pdata) throw std::logic_error("Data to be added onto queue is NULL");

	BOOLEAN      fresize        = FALSE;
	UINT32 const uiSize_of_each = q->uiSiz_of_elem;
	UINT32 const uiMax_size     = q->uiMax_size;
	UINT32 const uiHead         = q->uiHead;
	UINT32       uiTail         = q->uiTail;
	if (uiTail + uiSize_of_each > uiMax_size)
	{
		uiTail = q->uiTail = sizeof(QueueHeader);
		fresize = TRUE;
	}
	if (uiHead == uiTail && (uiHead >= sizeof(QueueHeader) + uiSize_of_each || fresize))
	{
		UINT32 const uiNew_size = 2 * uiMax_size - sizeof(QueueHeader);
		q->uiMax_size = uiNew_size;
		q = (HQUEUE)MemRealloc(q, uiNew_size);
		// copy memory from beginning of container to end of container
		// so that all the data is in one continuous block

		if (uiHead > sizeof(QueueHeader))
		{
			BYTE* const presize  = (BYTE*)q + sizeof(QueueHeader);
			BYTE* const pmaxsize = (BYTE*)q + uiMax_size;
			memmove(pmaxsize, presize, uiHead - sizeof(QueueHeader));
		}
		q->uiTail = uiMax_size + uiHead - sizeof(QueueHeader);
	}
	BYTE* const pbyte = (BYTE*)q + q->uiTail;
	memmove(pbyte, pdata, uiSize_of_each);
	q->uiTotal_items++;
	q->uiTail += uiSize_of_each;
	return q;
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
