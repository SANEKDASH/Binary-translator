#ifndef LIST_HEADER
#define LIST_HEADER

#include <stdio.h>
#include <string.h>

#include "../backend_common.h"
#include "../../debug/debug.h"
#include "../../debug/color_print.h"

#ifdef DEBUG

    #define TEXT_DUMP_LIST(list) TextDumpList(list)

    #define DEBUG_ON(...) __VA_ARGS__

    #define LIST_VERIFY(list) ListVerify(list)

#else

    #define TEXT_DUMP_LIST(list) ;

    #define DEBUG_ON(...) ;

    #define LIST_VERIFY(list) ;

#endif

typedef Instruction ListElemType_t;

typedef unsigned int ListState_t;

struct List
{
    ListElemType_t *data;

    int *prev;
    int *next;

    int head;
    int tail;
    int free;

    size_t capacity;
    size_t elem_count;

    ListState_t status;
};

typedef enum
{
    kListClear              = 1 << 0,
    kHeadLessZero           = 1 << 1,
    kTailLessZero           = 1 << 2,
    kListNullData           = 1 << 3,
    kCapacityLessZero       = 1 << 4,
    kTailLessHead           = 1 << 5,
    kSizeLessZero           = 1 << 6,
    kElemCountLessZero      = 1 << 7,
    kWrongPrev              = 1 << 8,
    kWrongTailNext          = 1 << 9,

    kWrongUsingOfList       = 1 << 10,
    kListFailedAllocation   = 1 << 11,
    kListFailedReallocation = 1 << 12,
    kListFailedToOpenFile   = 1 << 13,

    kListLogFileWasNotOpened,
    kListCantFind,
} ListErrs_t;

ListState_t ListVerify(List *list);

ListState_t ListConstructor(List *list);

ListState_t ListDestructor(List *list);

ListState_t ListAddAfter(List           *list,
                         size_t          pos,
                         ListElemType_t  value);

ListState_t ListFind(List           *list,
                     ListElemType_t  val,
                     size_t         *list_pos);

ListState_t ListAddBefore(List           *list,
                          size_t          pos,
                          ListElemType_t  value);

ListState_t ListDelete(List   *list,
                       size_t  pos);

ListState_t TextDumpList(List *list);

#endif
