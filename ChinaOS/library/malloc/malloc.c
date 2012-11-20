/******************************************************Copyright (c)**************************************************
**
**                              Copyright (c) 2009,2011 Fengliang And His Friends Corporation.
**
**                                             E-Mail: ChinaFengliang@163.com
**
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            malloc.c
** Last version:         V1.00
** Descriptions:         内存分配器机制实现文件.
** Hardware platform:    
** SoftWare platform:    基于2.8.4版本修改.
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011年3月20日  18:20:14
** Version:              V1.00
** Descriptions:         The original version
**
**--------------------------------------------------------------------------------------------------------------------
** Modified by: 
** Modified date:
** Version:
** Descriptions:
**
*********************************************************************************************************************/
#define DEBUG_LOCAL_EN                              1                       /* 本地调试开关(默认配置): 0:关; 1:开   */
/*********************************************************************************************************************
                                                    头文件区
*********************************************************************************************************************/
#include    <string.h>
#include    <include/macro.h>
#include    <include/types.h>
#include    <include/debug.h>
#include    <kernel/semaphore/semaphore.h>
#include    <library/misc/misc.h>
#include    <library/ioport.h>
#include    <library/bit/bit.h>


/*********************************************************************************************************************
                                                    开发笔记
*********************************************************************************************************************/
/*
 |- 1) 内存池
 |       本内存分配器支持对多个内存段进行管理.
 |                                           << 内存池镜像 >>
 |                   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ low
 |       StartAddr ->|                          内存池空间                           .
 |                   .                                                               .
 |                   .                                                               .
 |                   .                                                               .
 |                   .                                                               .
 |                   .                                                               .
 |                   .                                                               |
 |                   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ high
 |        EndAddr  ->|                         未知内存空间                          |
 |                   .                                                               .
 |                   .                                                               .                             
 |                   +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 |       注: 
 |           内存区间: [StartAddr, EndAddr)
 |
 |- 2) 内存块
 |       内存块是分配器可分配的最小元素.
 |                                           << 内存块镜像 >>
 |                   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ low
 |         Chunk  -> |             Size of previous chunk                          |b|
 |                   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |             head: |             Size of chunk, in bytes                         |b|
 |                   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |             mem-> |             Forward pointer to next chunk in list             |
 |                   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                   |             Back pointer to previous chunk in list            |
 |                   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 |                   |             Unused space (may be 0 bytes long)                |
 |                   .                                                               .
 |                   .                                                               .
 |                   .                                                               .
 |                   .                                                               .
 |                   .                                                               .
 |                   |                                                               |
 |                   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ high
 |
 |       注: 
 |           b == 0: 空闲(FREE);
 |           b == 1: 占用(USED);
 |
 */

/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 宏段名 ----------------------------------------------------------------------------------------------------------*/
#define FREE                            0ul                                 /* 空闲                                 */
#define USED                            1ul                                 /* 占用                                 */
#define MIN_CHUNK_SIZE                 16ul                                 /* 最小chunk内存空间大小                */

/* mask with all bits to left of least bit of x on */
#define left_bits(x)         ((x<<1) | -(x<<1))

/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 内存池类型 ------------------------------------------------------------------------------------------------------*/
struct __segment
{
    void               *pStartAddr;                                         /* 内存段起始地址(包括)                 */
    void               *pEndAddr;                                           /* 内存段结束地址(不包括)               */
};
typedef struct __segment                            SEGMENT;                /* 内存段对象类型                       */

/* 内存池类型 ------------------------------------------------------------------------------------------------------*/
struct __chunk
{
    INT32U              PrevInfo;                                           /* 前块空间信息(大小 + 状态)            */
    INT32U              ThisInfo;                                           /* 本块空间信息(大小 + 状态)            */
    
    struct __chunk     *pFreeNext;                                          /* 下一空闲块地址                       */
    struct __chunk     *pFreePrev;                                          /* 上一空闲块地址                       */
};
typedef struct __chunk                              CHUNK;                  /* 内存块信息类型                       */

/* 内存分配器类型 --------------------------------------------------------------------------------------------------*/
struct __tree_node
{
    INT32U              PrevInfo;                                           /* 前块空间信息(大小 + 状态)            */
    INT32U              ThisInfo;                                           /* 本块空间信息(大小 + 状态)            */
    struct __tree_node *pFreeNext;                                          /* 下一空闲块地址                       */
    struct __tree_node *pFreePrev;                                          /* 上一空闲块地址                       */

    struct __tree_node *pChild[2];                                          /* 二分叉子节点地址                     */
    struct __tree_node *pParent;                                            /* 父节点地址                           */
    INT32U              Index;                                              /* 树索引                               */
};
typedef struct __tree_node                          TNODE;                  /* 树节点类型                           */

/* 内存分配器类型 --------------------------------------------------------------------------------------------------*/
struct __memory_allocor
{
    INT32U              SameMap;                                            /* 同型内存容器组状态位图               */
    INT32U              TreeMap;                                            /* 树型内存容器组状态位图               */

    CHUNK              *pRecent;                                            /* 最近使用块地址                       */
    INT32U              SpareSize;                                          /* 剩余空间大小(因分配小块而剩余)       */

    CHUNK              *pSameChunks[32*2 + 2];                              /* 同型内存容器组                       */
    TNODE              *pTreeChunks[32];                                    /* 树型内存容器组                       */
};
typedef struct __memory_allocor                     MALLOCOR;               /* 内存分配器类型                       */

/*********************************************************************************************************************
                                                  全局变量定义区
*********************************************************************************************************************/
extern void                *Image$$HeapBase1$$Base;                         /* 堆区1基址                            */
extern void                *Image$$HeapBase2$$Base;                         /* 栈区2基址                            */
static MALLOCOR             Mallocor;                                       /* 内存分配器                           */
static struct __semaphore   AccessLock;                                     /* 访问锁                               */
static const SEGMENT        aMemorySegments[] =                             /* 内存段列表                           */
{
    {&Image$$HeapBase1$$Base, (void *)0x10008000},
    {&Image$$HeapBase2$$Base, (void *)0x20084000},
};


/*********************************************************************************************************************
** Function name:           heap
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-4  15:40:57
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int heap(char *Option)
{
    int         i;


    for (i = 0; i < sizeof(aMemorySegments)/sizeof(SEGMENT); i++)
    {
        printk("内存空间%d:[0x%X,0x%X] 共%dK字节\r\n", i, 
               (INT32U)aMemorySegments[i].pStartAddr, (INT32U)aMemorySegments[i].pEndAddr, 
               (((INT32U)aMemorySegments[i].pEndAddr - (INT32U)aMemorySegments[i].pStartAddr)>>10));
    }
    
    return OK;
}
EXPORT_TO_CONSOLE("内存空间", heap);

/*********************************************************************************************************************
** Function name:           get_chunk
** Descriptions:            获取同型内存容器信息
** Input parameters:        Index : 内存容器索引
** Output parameters:       
** Returned value:          头节点指针
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-23  0:49:25
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE CHUNK * get_chunk(INT32U Index)
{
    return (CHUNK *)&(Mallocor.pSameChunks[Index << 1]);
}

/*********************************************************************************************************************
** Function name:           chunk_to_memeory
** Descriptions:            转换内存块首址为应用内存首址
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-3-10  12:52:13
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE char * chunk_to_memeory(CHUNK *pChunk)
{
    return (char *)pChunk + (sizeof(INT32U)*2);                             /* 裁剪掉PrevInfo与ThisInfo标识空间     */
}

/*********************************************************************************************************************
** Function name:           get_box_index
** Descriptions:            计算box索引
** Input parameters:        Size : 应用空间大小(单位:字节)
** Output parameters:       
** Returned value:          对应box索引
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-3-10  15:56:21
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE INT32U get_box_index(INT32U Size)
{
    return (Size >> 3) - 1;                                                 /* 计算内存块对应的索引                 */
}

/*********************************************************************************************************************
** Function name:           tree_size_to_index
** Descriptions:            获取对应空间大小的树目录索引号
** Input parameters:        Size : 空间大小
** Output parameters:       
** Returned value:          树目录索引号
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-24  11:23:18
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE INT32U tree_size_to_index(INT32U Size)
{
    INT32U          Index;
    INT32U          x;


    x = Size >> 8;                                                          /* x = Size / 256                       */
    if (x == 0)                                                             /* [0,256)                              */
    {
        Index = 0;
    }
    else if (x > 0xFFFF)                                                    /* [16777216, 无穷大)                   */
    {
        Index = 31;
    }
    else
    {                                                                       /* [256, 16777216)                      */
        INT32U y = x;
        INT32U n = ((y - 0x100) >> 16) & 8;
        INT32U k = (((y <<= n) - 0x1000) >> 16) & 4;

        n += k;
        n += k = (((y <<= k) - 0x4000) >> 16) & 2;
        k = 14 - n + ((y <<= k) >> 15ul);
        Index = (k << 1) + ((Size >> (k + 7) & 1ul));
    }

    return Index;
}

/*********************************************************************************************************************
** Function name:           leftshift_for_tree_index
** Descriptions:            获取容量码至顶的左移位数
** Input parameters:        Index : 树索引号
** Output parameters:       
** Returned value:          需要左移位数
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-24  11:40:31
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE INT32U leftshift_for_tree_index(INT32U Index)
{
   return (Index == 31) ? 0 : (25 - (Index >> 1));
}

/*********************************************************************************************************************
** Function name:           get_chunk_size
** Descriptions:            获取chunk空间大小信息
** Input parameters:        pChunk : 内存块首地址
** Output parameters:       
** Returned value:          chunk空间大小
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-24  12:34:15
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE INT32U get_chunk_size(TNODE *pChunk)
{
    return pChunk->ThisInfo & ~1ul;
}

/*********************************************************************************************************************
** Function name:           insert_small_chunk
** Descriptions:            插入内存块信息到同类环形表
** Input parameters:        pChunk : 内存块首地址
**                          Size   : 应用内存大小
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-24  15:10:41
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE void insert_small_chunk(CHUNK *pChunk, INT32U Size)
{
    INT32U              Index;                                              /* 组索引                               */
    CHUNK              *pHead;                                              /* 头指针                               */
    

    Index = get_box_index(Size);    
    pHead = get_chunk(Index);
    
    if (0 == test_bit(Mallocor.SameMap, Index))
    {
        Mallocor.SameMap |= 1ul << Index;
    }

    /* 添加到环形链表的首节点 */
    pChunk->pFreeNext = pHead->pFreeNext;
    pChunk->pFreePrev = pHead;
    pHead->pFreeNext->pFreePrev = pChunk;
    pHead->pFreeNext  = pChunk;
}

/*********************************************************************************************************************
** Function name:           insert_large_chunk
** Descriptions:            插入内存块信息到树目录
** Input parameters:        pChunk : 内存块首地址
**                          Size   : 应用内存大小
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-24  11:25:52
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE void insert_large_chunk(TNODE *pChunk, INT32U Size)
{
    INT32U          Index;
    TNODE          *pNode;
    INT32U          BitMap;
    INT32U          ChunkSize;
    
    
    Index = tree_size_to_index(Size);
    pNode = Mallocor.pTreeChunks[Index];
 
    pChunk->Index = Index;
    pChunk->pChild[0] = pChunk->pChild[1] = NULL;

    /*
     * A) 空树
     */
    if (0ul == test_bit(Mallocor.TreeMap, Index))
    {
        Mallocor.TreeMap |= (1ul << Index); 
        Mallocor.pTreeChunks[Index] = pChunk;
        pChunk->pParent = (TNODE *)&(Mallocor.pTreeChunks[Index]);
        pChunk->pFreeNext = pChunk->pFreePrev = pChunk;                     /* 单独构成一个同型环形                 */
        return;
    }

    /*
     * B)非空树
     */
    BitMap = Size << leftshift_for_tree_index(Index);
    ChunkSize = Size + 8;
    while (1)
    {
        if (ChunkSize != get_chunk_size(pNode))
        {
            TNODE **C = &(pNode->pChild[(BitMap >> 31) & 1ul]);
            BitMap <<= 1;

            if (NULL != *C)                                                 /* 如果存在子分枝时                     */
            {
                /* move into son of the tree */
                pNode = *C;                                                 /* 进入树分枝                           */
            }
            else
            {
                /*
                 * here we get the end of tree, so link pChunk as a leaf node.
                 */
                *C = pChunk;
                pChunk->pParent   = pNode;
                pChunk->pFreeNext = pChunk;
                pChunk->pFreePrev = pChunk;
                break;
            }
        }
        else
        {
            /*
             * pChunk has same size with pNode, and only the first node has
             * parent identify.
             */
            pChunk->pParent   = NULL;

            pChunk->pFreeNext = pNode->pFreeNext;
            pChunk->pFreePrev = pNode;
            pNode->pFreeNext->pFreePrev = pChunk;
            pNode->pFreeNext  = pChunk;            
    
            break;
        }
    }
}

/*********************************************************************************************************************
** Function name:           is_small
** Descriptions:            判断是否是小块
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-3-10  14:58:48
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE BOOL is_small(INT32U Size)
{
    return (Size <= 256);
}

/*********************************************************************************************************************
** Function name:           insert_chunk
** Descriptions:            
** Input parameters:        pChunk : 内存块
**                          ChunkSize : 内存块大小(包含首部信息空间)
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-28  18:6:42
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
static void insert_chunk(CHUNK *pChunk, INT32U ChunkSize)
{    
    INT32U          Size;


    Size = ChunkSize - 8;                                                   /* 去除标识符空间大小                   */
    
    if (is_small(Size))
    {
        insert_small_chunk(pChunk, Size);
    }
    else 
    { 
        TNODE *pTreeNode = (TNODE *)pChunk;
        insert_large_chunk(pTreeNode, Size);
    }
}

/*********************************************************************************************************************
** Function name:           update_temporary_chunk
** Descriptions:            更新临时块
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-26  18:11:39
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void update_temporary_chunk(CHUNK *pChunk, INT32U Size)
{
    if (Mallocor.SpareSize)
    {
        insert_chunk(Mallocor.pRecent, Mallocor.SpareSize);
    }
    Mallocor.pRecent   = pChunk;
    Mallocor.SpareSize = Size;
}

INLINE TNODE *letfmost_child(TNODE *pTreeNode)
{
    return pTreeNode->pChild[0] != NULL ? pTreeNode->pChild[0] : pTreeNode->pChild[1];
}

/*********************************************************************************************************************
** Function name:           unlink_small_chunk
** Descriptions:            Unlink a chunk from a smallbin
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-4-8  16:34:37
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE void unlink_small_chunk(CHUNK *pChunk, INT32U Size)
{
    CHUNK       *FreeNext = pChunk->pFreeNext;
    CHUNK       *FreePrev = pChunk->pFreePrev;
    INT32U       Index    = get_box_index(Size);


    if (FreeNext == FreePrev)
    {
        Mallocor.SameMap &= ~(1ul << Index);
    }

    FreeNext->pFreePrev = FreePrev;
    FreePrev->pFreeNext = FreeNext;
}

/*********************************************************************************************************************
** Function name:           unlink_large_chunk
** Descriptions:            删除大块在树目录中的信息节点
** Input parameters:        pChunk : 目标大块首址
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-28  17:0:44
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE void unlink_large_chunk(TNODE* pChunk)
{    
    TNODE               *pParent;
    TNODE               *pReplace;                                          /* 替代节点                             */

    
    if (pChunk->pFreePrev != pChunk)
    {
        /* 1) If x is a chained node, unlink it from its same-sized fd/bk links
         *    and choose its bk node as its replacement.
         */
        TNODE *pNext = pChunk->pFreeNext;

        pReplace = pChunk->pFreePrev;
        pNext->pFreePrev = pReplace;
        pReplace->pFreeNext = pNext;
    }
    else
    {
        /* 2) If x was the last node of its size, but not a leaf node, it must
         *    be replaced with a leaf node (not merely one with an open left or
         *    right), to make sure that lefts and rights of descendents
         *    correspond properly to bit masks.  We use the rightmost descendent
         *    of x.  We could use any other leaf, but this is easy to locate and
         *    tends to counteract removal of leftmosts elsewhere, and so keeps
         *    paths shorter than minimally guaranteed.  This doesn't loop much
         *    because on average a node in a tree is near the bottom. */
        TNODE **ppNode;
        if ((NULL != (pReplace = *(ppNode = &(pChunk->pChild[1])))) || 
            (NULL != (pReplace = *(ppNode = &(pChunk->pChild[0])))))
        {
            TNODE **ppTmp;
            while((NULL != *(ppTmp = &(pReplace->pChild[1]))) || 
                  (NULL != *(ppTmp = &(pReplace->pChild[0]))))
            {
                pReplace = *(ppNode = ppTmp);
            }

            *ppNode = NULL;
        }
    }

    /* 3) If x is the base of a chain (i.e., has parent links) relink
     *    x's parent and children to x's replacement (or null if none).*/
    pParent = pChunk->pParent;
    if (NULL != pParent)
    {
        TNODE **ppTmp = &(Mallocor.pTreeChunks[pChunk->Index]);
        
        if (pChunk == *ppTmp)
        {
            if (NULL == (*ppTmp = pReplace))
            {
                Mallocor.TreeMap &= ~(1ul << pChunk->Index);
            }
        }
        else
        {
            if (pParent->pChild[0] == pChunk)
            {
                pParent->pChild[0] = pReplace;
            }
            else
            {
                pParent->pChild[1] = pReplace;
            }
        }

        if (NULL != pReplace)
        {
            TNODE *pChild_0, *pChild_1;
            
            pReplace->pParent = pParent;

            if (NULL != (pChild_0 = pChunk->pChild[0]))
            {
                pReplace->pChild[0] = pChild_0;
                pChild_0->pParent   = pReplace;
            }

            if (NULL != (pChild_1 = pChunk->pChild[1]))
            {
                pReplace->pChild[1] = pChild_1;
                pChild_1->pParent   = pReplace;
            }
        }
    }
}

/*********************************************************************************************************************
** Function name:           unlink_chunk
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-4-8  17:11:33
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE void unlink_chunk(CHUNK *pChunk, INT32U Size)
{
    if (is_small(Size))
    {

        unlink_small_chunk(pChunk, Size);
    }
    else
    {
        unlink_large_chunk((TNODE *)pChunk);
    }
}

/*********************************************************************************************************************
** Function name:           malloc_small_in_tree
** Descriptions:            寻找树目录中最小的节点
** Input parameters:        Size : 空间大小(单位: Byte)
** Output parameters:       
** Returned value:          == NULL : 申请失败
**                          != NULL : 内存块用户区首址
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-20  18:50:52
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
static void* malloc_small_in_tree(INT32U Size)
{
    INT32U              Index;                                              /* 索引                                 */
    INT32U              ChunkSize;                                          /* 内存块空间                           */
    INT32U              FreeSize;                                           /* 最小差额                             */
    TNODE              *pTmp, *pChunk, *pNext;
    
    
    if (0 == Mallocor.TreeMap)
    {
        return NULL;
    }

    /*
     * 1) 锁定最小箱索引
     */
    Index  = bit_scan_forward(Mallocor.TreeMap);
    pChunk = pTmp = Mallocor.pTreeChunks[Index];
    FreeSize  = pTmp->ThisInfo;                                             /* 因为FREE位: 0                        */

    /*
     * 2) 搜索最小块信息
     */
    while (NULL != (pTmp = letfmost_child(pTmp)))
    {
        INT32U ThisMargin = pTmp->ThisInfo;                                 /* 本次差额                             */

        if (ThisMargin < FreeSize)
        {
            FreeSize = ThisMargin;
            pChunk   = pTmp;
        }
    }

    /*
     * 3) 拆分最小块空间
     */
    ChunkSize = pChunk->ThisInfo;
    pNext = (TNODE *)((INT32U)pChunk + ChunkSize);
    unlink_large_chunk(pChunk);
    if (FreeSize - Size < MIN_CHUNK_SIZE)
    {
        /*
         * 独占天下
         */
        pChunk->ThisInfo = pNext->PrevInfo  = ChunkSize | USED;
    }
    else
    {
        /*
         * 天下二分
         */
        TNODE *pFree = (TNODE *)((char *)pChunk + Size);
        pFree->ThisInfo = pNext->PrevInfo = FreeSize - Size;
        pChunk->ThisInfo = pFree->PrevInfo = Size | USED;

        update_temporary_chunk((CHUNK *)pFree, FreeSize - Size);
    }
    
    return (char *)pChunk + MIN_CHUNK_SIZE;
}

/*********************************************************************************************************************
** Function name:           tmalloc_large
** Descriptions:            allocate a large request from the best fitting chunk in a treebin
** Input parameters:        Size : 空间大小(单位: Byte)
** Output parameters:       
** Returned value:          ==NULL : 操作失败
**                          !=NULL : 操作成功(内存空间地址)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-20  18:51:29
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
static void * malloc_large_in_tree(INT32U Size) 
{
    INT32U       rsize = -Size;                                             /* 最小差距                             */
    TNODE       *pChunk = NULL;                                             /* 最佳块                               */
    TNODE       *pTree;
    INT32U       idx;


    idx = tree_size_to_index(Size);
    if (NULL != (pTree = Mallocor.pTreeChunks[idx]))
    {
        INT32U sizebits = Size << leftshift_for_tree_index(idx);
        TNODE  *rst = NULL;

        while(1)
        {
            TNODE   *rt;
            INT32U   trem = get_chunk_size(pTree) - Size;                   /* 本次差距                             */
            
            if (trem < rsize)
            {
                pChunk = pTree;
                if (0 == (rsize = trem))
                {
                    break;
                }
            }
            
            rt = pTree->pChild[1];
            pTree  = pTree->pChild[(sizebits >> 31) & 1ul];
            
            if (rt != NULL && rt != pTree)
            {
                rst = rt;
            }
            
            if (NULL == pTree)
            {
                pTree = rst;
                break;
            }
            
            sizebits <<= 1;
        }
    }

    /*
     * set t to root of next non-empty treebin
     */
    if (NULL == pTree && NULL == pChunk)
    {   
        INT32U leftbits = Mallocor.TreeMap & left_bits(1ul << idx);
        if (leftbits != 0)
        {
            INT32U Index;
            Index = bit_scan_forward(leftbits);
            pTree = Mallocor.pTreeChunks[Index];
        }
    }

    /*
     * find smallest of tree or subtree
     */
    while (NULL != pTree)                                                       
    {   
        INT32U trem = get_chunk_size(pTree) - Size;
        if (trem < rsize)
        {
            rsize = trem;
            pChunk = pTree;
        }
        pTree = letfmost_child(pTree);
    }

    /*
     * 与dv比较 
     */
    if (NULL != pChunk && rsize < (Mallocor.SpareSize - Size))
    {
        TNODE *pNext = (TNODE *)((char *)pChunk + rsize + Size);

        unlink_large_chunk(pChunk);
        if (rsize < MIN_CHUNK_SIZE)
        {
            pChunk->ThisInfo = pNext->PrevInfo = (rsize + Size) | USED;
        }
        else
        {
            TNODE *pFree = (TNODE *)((char *)pChunk + Size);

            pFree->ThisInfo  = pNext->PrevInfo = rsize;
            pChunk->ThisInfo = pFree->PrevInfo = Size | USED;

            insert_chunk((CHUNK *)pFree, rsize);
        }
        return (char *)pChunk + MIN_CHUNK_SIZE; 
    }
    
    return NULL;
}

/*********************************************************************************************************************
** Function name:           unlink_first_small_chunk
** Descriptions:            Unlink the first chunk from a smallbin 
** Input parameters:        pHead  : 首节点
**                          pChunk : 目标节点
**                          Index  : 箱索引号
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-26  16:45:15
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE void unlink_first_small_chunk(CHUNK *pHead, CHUNK *pChunk, INT32U Index)
{
    CHUNK               *pNext;

    pNext = pChunk->pFreeNext;
    
    if (pHead == pNext)                                                     /* 仅存在一个空闲chunk                  */
    {
        Mallocor.SameMap &= ~(1ul << Index);
    }

    pHead->pFreeNext = pNext;
    pNext->pFreePrev = pHead;
}
    
/*********************************************************************************************************************
** Function name:           mallocor_setup
** Descriptions:            安装内存分配器
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-20  18:30:6
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
STATUS mallocor_setup(void)
{
    int             i;

    /*
     * 1) 创建管理信号量
     */    
    AccessLock.Counter = 1;
    AccessLock.pThread = NULL;
    
    memset(&Mallocor, 0, sizeof(Mallocor));                                 /* 清零分配器管理信息                   */

    /*
     * 2) 初始化内存容器组
     */
    for (i = 0; i < 32; i++)
    {
        CHUNK *pChunk = get_chunk(i);
        pChunk->pFreeNext = pChunk;                                         /* 组织环形链表头节点                   */
        pChunk->pFreePrev = pChunk;
    }

    /*
     * 3) 注册所有内存池
     */
    for (i = 0; i < ARRAY_SIZE(aMemorySegments); i++)
    {
        INT32U      StartAddr;                                              /* 有效空间起始地址                     */
        INT32U      EndAddr;                                                /* 有效空间结束地址                     */
        INT32U      ChunkSize;                                              /* chunk空间大小                        */
        
        StartAddr = (INT32U)aMemorySegments[i].pStartAddr;
        EndAddr   = (INT32U)aMemorySegments[i].pEndAddr;             

        /* 对齐到ARM系统字对齐格式 */
        StartAddr = align_upside(StartAddr, sizeof(INT32U));
        EndAddr   = align_downside(EndAddr, sizeof(INT32U));
        
        EndAddr  -= sizeof(INT32U) << 1;                                    /* 减去尾部块标识符空间位置             */
        ChunkSize = EndAddr - StartAddr;
        
        /*
         * 3.1) 设定内存块标识符信息
         *   a)标识前一块chunk空间为0字节,且占用.以做为前面边界标识符.
         *   b)标识后一块chunk空间为0字节,且占用.以做为后面边界标识符.
         */
        write_dword(StartAddr, 0ul | USED);                                 /* PrevInfo                             */
        write_dword(StartAddr + sizeof(INT32U), ChunkSize | FREE);          /* ThisInfo                             */        
        write_dword(EndAddr, ChunkSize | FREE);                             /* PrevInfo                             */
        write_dword(EndAddr + sizeof(INT32U), 0ul | USED);                  /* ThisInfo                             */

        /*
         * 3.2) 将内存块插入到分配表
         */
        insert_chunk((CHUNK *)StartAddr, ChunkSize);
    }

    return OK;
}

/*********************************************************************************************************************
** Function name:           malloc
** Descriptions:            分配内存空间
** Input parameters:        Size : 申请内存空间大小(单位: Byte)
** Output parameters:       
** Returned value:          ==NULL : 操作失败
**                          !=NULL : 操作成功(内存空间地址)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-20  18:1:49
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void* malloc(INT32U Size)
{
    void            *pAppMemory;                                            /* 应用内存区首址                       */
    INT32U           RemainSize;                                            /* 残存空间大小                         */
    CHUNK           *pChunk;
    CHUNK           *pNext;
     
    semaphore_wait(&AccessLock, 0);
    Size = align_upside(Size + MIN_CHUNK_SIZE, sizeof(int));                /* 格式化申请尺寸                       */
    
    if (Size <= 256) 
    {
        INT32U           Index;                                             /* 内存箱索引号                         */
        INT32U           SameBits;                                          /* 同型内存容器组状态位图               */
       
        Size       = align_upside(Size, 8);
        Index      = get_box_index(Size);
        SameBits   = Mallocor.SameMap >> Index;                             /* 去除小于不满足的索引位               */
        
        /* 
         * 1) 狭义分配内存空间
         *    在邻近空闲箱中分配内存单元
         */
        if (0 != (SameBits & 0x03))
        {
            CHUNK           *pHead;
            
            Index  = ~SameBits & 1ul;
            pHead  = get_chunk(Index);                                      /* 获取环形链表头节点                   */
            pChunk = pHead->pFreeNext;
            unlink_first_small_chunk(pHead, pChunk, Index);
            /* 修改内存镜像 */
            pNext = (CHUNK *)((char *)pChunk + Size);
            pChunk->ThisInfo = pNext->PrevInfo = Size | USED;

            pAppMemory = chunk_to_memeory(pChunk);
            goto exit;
        }
        
        /*
         * 2) 局部分配内存空间
         *    从快速缓冲区中分配内存空间,以此保证局部聚合效应.
         */
        if (Size <= Mallocor.SpareSize)
        {
            goto quick;
        }

        /*
         * 3) 广义分配内存空间
         *    从更大的同类内存箱中分配内存块.
         */
        if (0 != SameBits)
        {   
            CHUNK           *pHead;
            
            SameBits <<= Index;
            Index = bit_scan_forward(SameBits);
            RemainSize = (Index << 3) - Size;
            pHead = get_chunk(Index);
            pChunk = pHead->pFreeNext;
            unlink_first_small_chunk(pHead, pChunk, Index);

            /* 
             * 由于从第1)层分配到广义分配最少多出8个字节,即最少多出一个
             * chunk空间, 所以此处直接断然将chunk一分为二 
             */
            pNext = (CHUNK *)((char *)pChunk + Size);
            pChunk->ThisInfo = pNext->PrevInfo = Size | USED;
            pNext->ThisInfo  = RemainSize | FREE;
            update_temporary_chunk(pNext, RemainSize);

            pAppMemory = chunk_to_memeory(pChunk);
            goto exit;
        }

        /*
         * 4) 在树目录结构中分配小块内存空间 
         */
        pAppMemory = malloc_small_in_tree(Size);
        goto exit;
    }

    /*
     * 5) 在树目录结构中分配大块内存空间
     */
    if (NULL != (pAppMemory = malloc_large_in_tree(Size)))
    {
        goto exit;
    }
    

    if (Mallocor.SpareSize < Size)
    {
        goto exit;
    }
        
    /*
     * 2) 局部分配内存空间 
     */    
quick:
    RemainSize = Mallocor.SpareSize - Size;
    pChunk     = Mallocor.pRecent;
    
    if (RemainSize < MIN_CHUNK_SIZE)
    {
        /*
         * 当被裁剪剩余的内存不足以存储最小内存块时, 所兴把所有的内存块
         * 分配出去.
         * 注意: 内存块空间尺寸是SpareSize.
         */
        pNext = (CHUNK *)((char *)pChunk + Mallocor.SpareSize);             /* 下一个内存块                         */
        pChunk->ThisInfo = pNext->PrevInfo = Mallocor.SpareSize | USED;
        Mallocor.SpareSize = 0;
        Mallocor.pRecent = NULL;
    }
    else
    {
        /*
         * 当被裁剪剩余的内存大于存储最小内存块时, 将分配目标尺寸的内存块,并
         * 将裁剪剩余的内存块保存在快速分配中.
         * 注意: 内存块空间尺寸是Size.
         */
        pNext = (CHUNK *)((char *)pChunk + Size);
        pChunk->ThisInfo = pNext->PrevInfo = Size | USED;
        Mallocor.pRecent = pNext;
        Mallocor.SpareSize = RemainSize;
    }
    
    pAppMemory = chunk_to_memeory(pChunk);
    
exit:
    semaphore_post(&AccessLock);
    return pAppMemory;
}

/*********************************************************************************************************************
** Function name:           free
** Descriptions:            释放内存空间
** Input parameters:        pMemory : 内存空间地址
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-20  18:3:31
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void free(void * pMemory)
{
    INT32U           Size;                                                  /* 本内存块大小                         */
    CHUNK           *pThisChunk;                                            /* 当前内存块                           */
    CHUNK           *pNextChunk;                                            /* next内存块                           */
    CHUNK           *pPrevChunk;                                            /* prev内存块                           */

    
    if (NULL == pMemory)
    {
        return;
    }
    
    semaphore_wait(&AccessLock, 0);                                         /* 线程互斥保护                         */

    pThisChunk = (CHUNK *)((char *)pMemory - MIN_CHUNK_SIZE);
    Size = pThisChunk->ThisInfo & ~1ul;
    pNextChunk = (CHUNK *)((char *)pThisChunk + Size);

    /*
     * 1) 对prev块进行处理
     */
    if (FREE == test_bit(pThisChunk->PrevInfo, 0))                          /* 测试上一块是否空闲 ?                 */
    {   /* A) prev块空闲状态 */
        INT32U    PrevSize = pThisChunk->PrevInfo;                          /* prev块大小                           */

        pPrevChunk = (CHUNK *)((char *)pThisChunk - PrevSize);              /* prev内存块                           */
        Size += PrevSize;
        pThisChunk = pPrevChunk;

        if(pThisChunk != Mallocor.pRecent)
        {
            unlink_chunk(pThisChunk, PrevSize);
        }
        else
        {
            if(FREE != test_bit(pNextChunk->ThisInfo, 0))                   /* 如果prev块就是dv块,且next块在使用中  */
            {   
                Mallocor.SpareSize = Size;
                pThisChunk->ThisInfo = pPrevChunk->PrevInfo = Size;                
            }
        }
    }

    /*
     * 2) 对next块进行处理
     */
    if (FREE == test_bit(pNextChunk->ThisInfo, 0))                          /* 测试下一块是否空闲 ?                 */
    {   /* 2.A) next块处于空闲状态下 */
        if (pNextChunk == Mallocor.pRecent)
        {
            INT32U dsize = Mallocor.SpareSize += Size;
            Mallocor.pRecent = pThisChunk;            
            pNextChunk = (CHUNK *)((char *)pThisChunk + dsize);
            pThisChunk->ThisInfo = pNextChunk->PrevInfo = dsize;
            goto exit;
        }
        else
        {
            INT32U nsize = pNextChunk->ThisInfo;
            Size += nsize;
            unlink_chunk(pNextChunk, nsize);
            pNextChunk = (CHUNK *)((char *)pNextChunk + nsize);
            pThisChunk->ThisInfo = pNextChunk->PrevInfo = Size;
            if (pThisChunk == Mallocor.pRecent)
            {
                Mallocor.SpareSize = Size;
                goto exit;
            }
        }
    }
    else
    {   /* 2.B) next块处于占用状态下 */
        pThisChunk->ThisInfo = pNextChunk->PrevInfo = Size;
    }

    /*
     * 3) 将内存块插入列表
     */
    insert_chunk(pThisChunk, Size);
    
exit:
    semaphore_post(&AccessLock);
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

