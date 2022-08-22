/*
 *	File: iAP2ListArray.h
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

/*********************************************************
 *
 *  ListArray buffer structure:
 *  (as presented, supports upto 255 entries (index 0 to 254) of upto 253 bytes of data)
 *
 *  msb                             lsb
 *   +---+---+---+---+---+---+---+---+
 *   | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 *   +---+---+---+---+---+---+---+---+
 *   |             Count             | byte 0
 *   +-------------------------------+
 *   |            Max Count          | byte 1
 *   +-------------------------------+
 *   |             Root              | byte 2   --------+
 *   +-------------------------------+                  |
 *   |             Last              | byte 3           |
 *   +-------------------------------+                  |
 *   |             Free              | byte 4           |
 *   +-------------------------------+                  |
 *   |            Data Size          | byte 5           |
 *   +-------------------------------+                  |
 *   |            Reserved           | byte 6           |
 *   +-------------------------------+                  |
 *   |            Reserved           | byte 7           |
 *   +-------------------------------+                  |
 *   |               .               |  ---Next---------|-------+
 *   |               .               |                  |       |
 *   |        First Item Data        | byte 6 to N  <---+---+   |
 *   |               .               |                      |   |
 *   |               .               |                      |   |
 *   +-------------------------------+                      |   |
 *   |               .               |  ---Prev-------------+   |
 *   |               .               |                          |
 *   |        Second Item Data       | byte (N + 1) to M  <-----+
 *   |               .               |
 *   |               .               |
 *   +-------------------------------+
 *   |               .               |
 *   |               .               |
 *
 *
 *  ListArray item data structure:
 *
 *   +-------------------------------+
 *   |           Item Next           | byte 0
 *   +-------------------------------+
 *   |           Item Prev           | byte 1
 *   +-------------------------------+
 *   |               .               |
 *   |               .               |
 *   |           Value Data          | byte 2 to N
 *   |               .               |
 *   |               .               |
 *   +-------------------------------+
 *
 *********************************************************/

#ifndef iAP2Utility_iAP2ListArray_h
#define iAP2Utility_iAP2ListArray_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push,1)
typedef struct {
    uint8_t next;
    uint8_t prev;

} iAP2ListArrayItemHeader_t;

typedef struct {
    uint8_t             count;
    uint8_t             maxCount;
    uint8_t             root;
    uint8_t             last;
    uint8_t             free;
    uint8_t             dataSize;
    uint8_t             reserved1;
    uint8_t             reserved2;

} iAP2ListArrayHeader_t;
#pragma pack(pop)

enum
{
    kiAP2ListArrayInvalidOffset         = -1,
    kiAP2ListArrayMaxCount              = 255,
    kiAP2ListArrayInvalidIndex          = kiAP2ListArrayMaxCount,
    kiAP2ListArrayMaxDataSize           = 253
};


/* Returns <0 if a < b, 0 if a = b, >0 if a > b */
typedef intptr_t (*piAP2ListArrayCompareFunc)(void* a, void* b, uint8_t dataSize);

typedef void (*piAP2ListArrayFunc)(void* item);
typedef void (*piAP2ListArrayIndexParamFunc)(void* item, uint8_t index, void* param);

/* Just does straight compare on pointer values. */
intptr_t iAP2ListArrayStraightCompare(void* a, void* b);


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
uint32_t iAP2ListArrayGetBuffSize (uint8_t maxCount, uint8_t dataSize);


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
**      listArrayBuffer:    list is initialized
**
**  Return:
**      int     0 if success, else non-zero
**
****************************************************************
*/
int iAP2ListArrayInit (uint8_t* buff, uint8_t maxCount, uint8_t dataSize);


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
**  Note:   cleanup function is called for each item data and the whole list
**          is reset.
**
****************************************************************
*/
void iAP2ListArrayCleanup (uint8_t* listArrayBuffer, piAP2ListArrayFunc itemCleanupFunc);


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
                           piAP2ListArrayFunc func);


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
                                            void*                        param);


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
                               piAP2ListArrayCompareFunc func);


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
void* iAP2ListArrayItemForIndex (uint8_t* listArrayBuffer, uint8_t index);


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
uint8_t iAP2ListArrayGetCount (uint8_t* listArrayBuffer);


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
uint8_t iAP2ListArrayGetFirstItemIndex (uint8_t* listArrayBuffer);


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
uint8_t iAP2ListArrayGetLastItemIndex (uint8_t* listArrayBuffer);


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
uint8_t iAP2ListArrayGetNextItemIndex (uint8_t* listArrayBuffer, uint8_t itemIndex);


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
                                   void*    item);


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
                                     piAP2ListArrayCompareFunc func);


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
                              piAP2ListArrayFunc    itemCleanupFunc);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef iAP2Utility_iAP2ListArray_h */
