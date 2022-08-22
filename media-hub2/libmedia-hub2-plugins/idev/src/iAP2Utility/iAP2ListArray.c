/*
 *	File: iAP2ListArray.c
 *	Package: iAP2Utility
 *	Abstract: n/a 
 *
 *	Disclaimer: IMPORTANT: This Apple software is supplied to you, by Apple
 * 	Inc. ("Apple"), in your capacity as a current, and in good standing,
 *	Licensee in the MFi Licensing Program. Use of this Apple software is
 *	governed by and subject to the terms and conditions of your MFi License,
 *	including, but not limited to, the restrictions specified in the provision
 *	entitled “Public Software”, and is further subject to your agreement to
 *	the following additional terms, and your agreement that the use,
 *	installation, modification or redistribution of this Apple software
 * 	constitutes acceptance of these additional terms. If you do not agree with
 * 	these additional terms, please do not use, install, modify or redistribute
 *	this Apple software.
 *
 *	In consideration of your agreement to abide by the following terms, and
 *	subject to these terms, Apple grants you a personal, non-exclusive
 *	license, under Apple's copyrights in this original Apple software (the
 *	"Apple Software"), to use, reproduce, and modify the Apple Software in
 *	source form, and to use, reproduce, modify, and redistribute the Apple
 *	Software, with or without modifications, in binary form. While you may not
 *	redistribute the Apple Software in source form, should you redistribute
 *	the Apple Software in binary form, in its entirety and without
 *	modifications, you must retain this notice and the following text and
 *	disclaimers in all such redistributions of the Apple Software. Neither the
 *	name, trademarks, service marks, or logos of Apple Inc. may be used to
 *	endorse or promote products derived from the Apple Software without
 *	specific prior written permission from Apple. Except as expressly stated
 *	in this notice, no other rights or licenses, express or implied, are
 *	granted by Apple herein, including but not limited to any patent rights
 *	that may be infringed by your derivative works or by other works in which
 *	the Apple Software may be incorporated.
 *	
 *	The Apple Software is provided by Apple on an "AS IS" basis. APPLE MAKES
 *	NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
 *	IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
 *	PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
 *	ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 *
 *	IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
 *	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *	INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 *	MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND
 *	WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT
 *	LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY
 *	OF SUCH DAMAGE.
 *
 *	Copyright (C) 2012 Apple Inc. All Rights Reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "iAP2ListArray.h"
#include "iAP2Log.h"


/* Just does straight compare on pointer values. */
intptr_t iAP2ListArrayStraightCompare(void* a, void* b)
{
    intptr_t ai = (intptr_t)a;
    intptr_t bi = (intptr_t)b;
    return ai - bi;
}


static iAP2ListArrayItemHeader_t* _iAP2ListArrayItemFromIndex (iAP2ListArrayHeader_t* listArray,
                                                               uint8_t                index)
{
    if (listArray && index != kiAP2ListArrayInvalidIndex)
    {
        iAP2ListArrayItemHeader_t* itemList = (iAP2ListArrayItemHeader_t*) (listArray + 1);
        uint8_t* buff = (uint8_t*) itemList;
        iAP2ListArrayItemHeader_t* item;
        item = (iAP2ListArrayItemHeader_t*) (buff + (index * (sizeof(iAP2ListArrayItemHeader_t)
                                                              + listArray->dataSize)));
        return item;
    }
    return NULL;
}

static uint8_t* _iAP2ListArrayDataFromItem (iAP2ListArrayItemHeader_t* item)
{
    uint8_t* data = NULL;
    if (item)
    {
        data = (uint8_t*) (item + 1);
    }
    return data;
}

static uint8_t* _iAP2ListArrayDataFromItemIndex (iAP2ListArrayHeader_t* listArray,
                                                 uint8_t                index)
{
    uint8_t* data = NULL;
    if (listArray && index != kiAP2ListArrayInvalidIndex)
    {
        iAP2ListArrayItemHeader_t* item = _iAP2ListArrayItemFromIndex (listArray, index);
        data = _iAP2ListArrayDataFromItem (item);
    }
    return data;
}


/*
****************************************************************
**
**  iAP2ListArrayGetBuffSize
**
**  Input:
**      maxCount:   max number of items that can be stored
**      dataSize:   size of data that will be stored
**
**  Output:
**      None
**
**  Return:
**      uint32_t    minimum size of buff required for proper operation
**
****************************************************************
*/
uint32_t iAP2ListArrayGetBuffSize (uint8_t maxCount, uint8_t dataSize)
{
    uint8_t itemSize = dataSize + sizeof(iAP2ListArrayItemHeader_t);
    uint32_t result = sizeof(iAP2ListArrayHeader_t) + (maxCount * itemSize);
    assert(sizeof(iAP2ListArrayItemHeader_t) == 2);
    assert(sizeof(iAP2ListArrayHeader_t) == 8);
    return result;
}


/*
****************************************************************
**
**  iAP2ListArrayInit
**
**  Input:
**      listArrayBuffer:    buffer that will be used for iAP2ListArray
**                              Must be at least size returned by iAP2TimeGetBuffSize
**      maxCount:           max number of items that can be stored
**      dataSize:           size of data that will be stored
**
**  Output:
**      listArrayBuffer is initialized
**
**  Return:
**      int     0 if success, else non-zero
**
****************************************************************
*/
int iAP2ListArrayInit (uint8_t* buff, uint8_t maxCount, uint8_t dataSize)
{
    if (maxCount > 0 && maxCount <= kiAP2ListArrayInvalidIndex &&
        dataSize <= kiAP2ListArrayMaxDataSize)
    {
        uint8_t index;
        iAP2ListArrayHeader_t* header = (iAP2ListArrayHeader_t*) buff;
        iAP2ListArrayItemHeader_t* item;

        header->count       = 0;
        header->maxCount    = maxCount;
        header->root        = kiAP2ListArrayInvalidIndex;
        header->last        = kiAP2ListArrayInvalidIndex;
        header->free        = 0;
        header->dataSize    = dataSize;
        header->reserved1   = 0;
        header->reserved2   = 0;

        for (index = 0; index < maxCount; ++index)
        {
            item = _iAP2ListArrayItemFromIndex (header, index);
            item->next = index + 1;
            item->prev = index - 1;
        }
        item = _iAP2ListArrayItemFromIndex (header, 0);
        item->prev = kiAP2ListArrayInvalidIndex;
        item = _iAP2ListArrayItemFromIndex (header, header->maxCount - 1);
        item->next = kiAP2ListArrayInvalidIndex;

        return 0;
    }
    return -1;
}


/*
****************************************************************
**
**  iAP2ListArrayCleanup
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**      itemCleanupFunc:    function to call to cleanup data for each item
**
**  Output:
**      None
**
**  Return:
**      None
**
**  Note:   if provided, cleanup function is called for each item data and
**              the whole list is reset. (all nodes return to free list)
**
****************************************************************
*/
void iAP2ListArrayCleanup (uint8_t* listArrayBuffer, piAP2ListArrayFunc itemCleanupFunc)
{
    if (listArrayBuffer && itemCleanupFunc)
    {
        iAP2ListArrayHeader_t* header = (iAP2ListArrayHeader_t*) listArrayBuffer;
        uint8_t maxCount = header->maxCount;
        uint8_t dataSize = header->dataSize;

        if (itemCleanupFunc)
        {
            iAP2ListArrayForEach (listArrayBuffer, itemCleanupFunc);
        }
        iAP2ListArrayInit (listArrayBuffer, maxCount, dataSize);
    }
}


/*
****************************************************************
**
**  iAP2ListArrayForEachWithIndexAndParam
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**      func:               function to call for each item.
**      param:              parameter to pass to func
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2ListArrayForEachWithIndexAndParam (uint8_t*                     listArrayBuffer,
                                            piAP2ListArrayIndexParamFunc func,
                                            void*                        param)
{
/*     if (listArrayBuffer && func)
 *     {
 *         iAP2ListArrayHeader_t* header = (iAP2ListArrayHeader_t*) listArrayBuffer;
 *         uint8_t index = header->root;
 *         uint8_t count = header->count;
 *         if (count && index != kiAP2ListArrayInvalidIndex)
 *         {
 *             iAP2ListArrayItemHeader_t* item;
 *             uint8_t*                   data;
 *             
 *             while (index != kiAP2ListArrayInvalidIndex)
 *             {
 *                 item = _iAP2ListArrayItemFromIndex (header, index);
 *                 data = _iAP2ListArrayDataFromItem (item);
 *                 if (item && data)
 *                 {
 *                     func (data, (uint8_t) item, param);
 *                     index = item->next;
 *                 }
 *                 else
 *                 {
 *                     index = kiAP2ListArrayInvalidIndex;
 *                 }
 *             }
 *         }
 *     }
 */
}



/*
****************************************************************
**
**  iAP2ListArrayForEach
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**      func:               function to call for each item.
**
**  Output:
**      None
**
**  Return:
**      None
**
****************************************************************
*/
void iAP2ListArrayForEach (uint8_t*           listArrayBuffer,
                           piAP2ListArrayFunc func)
{
    if (listArrayBuffer && func)
    {
        iAP2ListArrayHeader_t* header = (iAP2ListArrayHeader_t*) listArrayBuffer;
        uint8_t index = header->root;
        uint8_t count = header->count;
        if (count && index != kiAP2ListArrayInvalidIndex)
        {
            iAP2ListArrayItemHeader_t* item;
            uint8_t*                   data;

            while (index != kiAP2ListArrayInvalidIndex)
            {
                item = _iAP2ListArrayItemFromIndex (header, index);
                data = _iAP2ListArrayDataFromItem (item);
                if (item && data)
                {
                    func (data);
                    index = item->next;
                }
                else
                {
                    index = kiAP2ListArrayInvalidIndex;
                }
            }
        }
    }
}


/*
****************************************************************
**
**  iAP2ListArrayFindItem
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**      item:               item to find.
**      func:               Compare function to compare items.
**
**  Output:
**      None
**
**  Return:
**      uint8_t index of found list item, kiAP2ListArrayInvalidIndex if not found.
**
****************************************************************
*/
uint8_t iAP2ListArrayFindItem (uint8_t*                  listArrayBuffer,
                               void*                     item,
                               piAP2ListArrayCompareFunc func)
{
    if (listArrayBuffer && func && item)
    {
        iAP2ListArrayHeader_t* header = (iAP2ListArrayHeader_t*) listArrayBuffer;
        uint8_t index = header->root;
        uint8_t count = header->count;

        if (count && index != kiAP2ListArrayInvalidIndex)
        {
            uint8_t dataSize = header->dataSize;

            while (index != kiAP2ListArrayInvalidIndex)
            {
                iAP2ListArrayItemHeader_t* pItem = _iAP2ListArrayItemFromIndex(header, index);
                uint8_t* data = _iAP2ListArrayDataFromItem (pItem);
                if (pItem && data)
                {
                    if (func (data, item, dataSize) == 0)
                    {
                        return index;
                    }
                    index = pItem->next;
                }
                else
                {
                    index = kiAP2ListArrayInvalidIndex;
                }
            }
        }
    }

    return kiAP2ListArrayInvalidIndex;
}


/*
****************************************************************
**
**  iAP2ListArrayItemForIndex
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**      index:              index to get item data for
**
**  Output:
**      None
**
**  Return:
**      void*   pointer to item data, NULL if not found.
**
****************************************************************
*/
void* iAP2ListArrayItemForIndex (uint8_t* listArrayBuffer, uint8_t index)
{
    if (listArrayBuffer && index != kiAP2ListArrayInvalidIndex)
    {
        iAP2ListArrayHeader_t* header = (iAP2ListArrayHeader_t*) listArrayBuffer;
        if (index < header->maxCount)
        {
            iAP2ListArrayItemHeader_t* item = _iAP2ListArrayItemFromIndex (header, index);
            return _iAP2ListArrayDataFromItem (item);
        }
    }

    return NULL;
}


/*
****************************************************************
**
**  iAP2ListArrayGetCount
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**
**  Output:
**      None
**
**  Return:
**      uint8_t count of items in list
**
****************************************************************
*/
uint8_t iAP2ListArrayGetCount (uint8_t* listArrayBuffer)
{
    if (listArrayBuffer)
    {
        iAP2ListArrayHeader_t* header = (iAP2ListArrayHeader_t*) listArrayBuffer;
        return header->count;
    }
    return 0;
}


/*
****************************************************************
**
**  iAP2ListArrayGetFirstItemIndex
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**
**  Output:
**      None
**
**  Return:
**      uint8_t index of first list item, kiAP2ListArrayInvalidIndex if nothing in list.
**
****************************************************************
*/
uint8_t iAP2ListArrayGetFirstItemIndex (uint8_t* listArrayBuffer)
{
    if (listArrayBuffer)
    {
        iAP2ListArrayHeader_t* header = (iAP2ListArrayHeader_t*) listArrayBuffer;
        return header->root;
    }
    return kiAP2ListArrayInvalidIndex;
}


/*
****************************************************************
**
**  iAP2ListArrayGetLastItemIndex
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**
**  Output:
**      None
**
**  Return:
**      uint8_t index of last list item, kiAP2ListArrayInvalidIndex if nothing in list.
**
****************************************************************
*/
uint8_t iAP2ListArrayGetLastItemIndex (uint8_t* listArrayBuffer)
{
    if (listArrayBuffer)
    {
        iAP2ListArrayHeader_t* header = (iAP2ListArrayHeader_t*) listArrayBuffer;
        return header->last;
    }
    return kiAP2ListArrayInvalidIndex;
}


/*
****************************************************************
**
**  iAP2ListArrayGetNextItemIndex
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**      itemIndex:          index of list item to get the next item index for.
**
**  Output:
**      None
**
**  Return:
**      uint8_t index of next list item, kiAP2ListArrayInvalidIndex if end of list.
**
****************************************************************
*/
uint8_t iAP2ListArrayGetNextItemIndex (uint8_t* listArrayBuffer, uint8_t itemIndex)
{
    if (listArrayBuffer)
    {
        iAP2ListArrayHeader_t* header = (iAP2ListArrayHeader_t*) listArrayBuffer;
        iAP2ListArrayItemHeader_t* pItem = _iAP2ListArrayItemFromIndex (header, itemIndex);
        if (pItem)
        {
            return pItem->next;
        }
    }
    return kiAP2ListArrayInvalidIndex;
}


/*
****************************************************************
**
**  iAP2ListArrayAddItemAfter
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**      prevItemIndex:      index of list item to add new item after.
**      item:               item to add.
**
**  Output:
**      listArrayBuffer:    list is updated to add new item
**
**  Return:
**      uint8_t index of added item.
**
**  Note:   If prev != kiAP2ListArrayInvalidIndex, new list node will be added behind prev.
**          If root != kiAP2ListArrayInvalidIndex and prev == kiAP2ListArrayInvalidIndex,
**              new item will be inserted at the beginning of the list.
**          If root == kiAP2ListArrayInvalidIndex and prev == kiAP2ListArrayInvalidIndex,
**              then the item becomes the first and only item in the list.
**
****************************************************************
*/
uint8_t iAP2ListArrayAddItemAfter (uint8_t* listArrayBuffer,
                                   uint8_t  prevItemIndex,
                                   void*    item)
{
    uint8_t itemIndex = kiAP2ListArrayInvalidIndex;
    iAP2ListArrayHeader_t* header = (iAP2ListArrayHeader_t*) listArrayBuffer;
    if (listArrayBuffer && item && header->count < header->maxCount)
    {
        uint8_t root = header->root;
        uint8_t last = header->last;
        uint8_t dataSize = header->dataSize;
        iAP2ListArrayItemHeader_t* pItem;
        iAP2ListArrayItemHeader_t* freeItem;
        uint8_t* data;

        /* Take an entry from the free list */
        itemIndex = header->free;
        assert (itemIndex != kiAP2ListArrayInvalidIndex);
        pItem = _iAP2ListArrayItemFromIndex (header, itemIndex);
        header->free = pItem->next;
        freeItem = _iAP2ListArrayItemFromIndex (header, header->free);
        if (freeItem)
        {
            freeItem->prev = kiAP2ListArrayInvalidIndex;
        }

        /* Update new item with passed in data */
        pItem->prev = kiAP2ListArrayInvalidIndex;
        pItem->next = kiAP2ListArrayInvalidIndex;
        data = _iAP2ListArrayDataFromItem (pItem);
        memcpy (data, item, dataSize);

        /* Insert item into list */
        if (prevItemIndex != kiAP2ListArrayInvalidIndex)
        {
            iAP2ListArrayItemHeader_t* prevItem = _iAP2ListArrayItemFromIndex (header, prevItemIndex);
            uint8_t prevNext = prevItem->next;
            if (prevNext != kiAP2ListArrayInvalidIndex)
            {
                iAP2ListArrayItemHeader_t* prevNextItem = _iAP2ListArrayItemFromIndex (header, prevNext);
                prevNextItem->prev = itemIndex;
            }
            pItem->next     = prevNext;
            prevItem->next  = itemIndex;
            pItem->prev     = prevItemIndex;
        }
        else
        {
            if (root != kiAP2ListArrayInvalidIndex)
            {
                iAP2ListArrayItemHeader_t* rootItem = _iAP2ListArrayItemFromIndex (header, root);
                rootItem->prev  = itemIndex;
                pItem->next     = root;
            }
            header->root = itemIndex;
        }
        if (prevItemIndex == last)
        {
            header->last = itemIndex;
        }
        header->count += 1;
    }
    return itemIndex;
}


/*
****************************************************************
**
**  iAP2ListArrayAddItemInOrder
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**      item:               item to add.
**      func:               Compare function to compare items.
**
**  Output:
**      listArrayBuffer:    list is updated to add new item
**
**  Return:
**      uint8_t index of added item.
**
**  Note:   Adds the item into the provided list in order determined
**          by the compare function. Ordering is ascending... ie. node is
**          inserted in between two nodes such that node before is <= 0 result
**          from comapre function with (before, node) and node after is >= 0 result
**          from compare function with (after, node).
**          If root == kiAP2ListArrayInvalidIndex,
**              then the item becomes the first and only item in the list.
**
****************************************************************
*/
uint8_t iAP2ListArrayAddItemInOrder (uint8_t*                  listArrayBuffer,
                                     void*                     item,
                                     piAP2ListArrayCompareFunc func)
{
    uint8_t itemIndex = kiAP2ListArrayInvalidIndex;
    if (listArrayBuffer && func && item)
    {
        iAP2ListArrayHeader_t* header = (iAP2ListArrayHeader_t*) listArrayBuffer;
        uint8_t index = header->root;
        uint8_t prev = kiAP2ListArrayInvalidOffset;
        uint8_t dataSize = header->dataSize;
        iAP2ListArrayItemHeader_t* foundItem = _iAP2ListArrayItemFromIndex(header, index);
        
        /* Find place to add item */
        while (foundItem &&
               index != kiAP2ListArrayInvalidIndex &&
               func (_iAP2ListArrayDataFromItem(foundItem), item, dataSize) <= 0)
        {
            prev = index;
            index = foundItem->next;
            foundItem = _iAP2ListArrayItemFromIndex(header, index);
        }

        itemIndex = iAP2ListArrayAddItemAfter (listArrayBuffer, prev, item);
    }

    return itemIndex;
}


/*
****************************************************************
**
**  iAP2ListArrayDeleteItem
**
**  Input:
**      listArrayBuffer:    buffer that is used for iAP2ListArray
**      index:              item to delete from list
**      itemCleanupFunc:    function to call to cleanup data for each item
**
**  Output:
**      listArrayBuffer:    list is updated to remove the item at index
**
**  Return:
**      None
**
**  Note:   Removes the item from the list and calls the itemCleanupFunc for
**              the removed item, if provided.
**
****************************************************************
*/
void iAP2ListArrayDeleteItem (uint8_t*              listArrayBuffer,
                              uint8_t               index,
                              piAP2ListArrayFunc    itemCleanupFunc)
{
    if (listArrayBuffer && index != kiAP2ListArrayInvalidIndex)
    {
        iAP2ListArrayHeader_t* header = (iAP2ListArrayHeader_t*) listArrayBuffer;
        uint8_t count = header->count;
        if (count > 0)
        {
            uint8_t root = header->root;
            uint8_t last = header->last;
            uint8_t free = header->free;
            iAP2ListArrayItemHeader_t* item = _iAP2ListArrayItemFromIndex (header, index);
            uint8_t next = item->next;
            uint8_t prev = item->prev;

            /* Remove item from list */
            if (index == last)
            {
                header->last = prev;
            }
            if (root == index)
            {
                header->root = next;
            }
            if (prev != kiAP2ListArrayInvalidIndex)
            {
                iAP2ListArrayItemHeader_t* prevItem = _iAP2ListArrayItemFromIndex (header, prev);
                prevItem->next = next;
            }
            if (next != kiAP2ListArrayInvalidIndex)
            {
                iAP2ListArrayItemHeader_t* nextItem = _iAP2ListArrayItemFromIndex (header, next);
                nextItem->prev = prev;
            }
            header->count -= 1;

            /* Cleanup the item data */
            if (itemCleanupFunc)
            {
                itemCleanupFunc (_iAP2ListArrayDataFromItem (item));
            }
            
            /* Add removed item to the free list */
            if (free != kiAP2ListArrayInvalidIndex)
            {
                iAP2ListArrayItemHeader_t* freeItem = _iAP2ListArrayItemFromIndex (header, free);
                freeItem->prev = index;
            }
            item->next   = free;
            header->free = index;
        }
    }
}


