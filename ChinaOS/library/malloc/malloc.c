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
#include    <library/memory.h>
#include    <library/ioport.h>
#include    <library/bit/bit.h>
#include    <library/link/list.h>


/*********************************************************************************************************************
                                                    开发笔记
*********************************************************************************************************************/
/*
 |- 1) 内存池
 |       本内存分配器支持对多个内存池进行管理.
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
 |           内存池区间: [StartAddr, EndAddr)
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
#define MIN_CHUNK_SIZE                  sizeof(CHUNK)                       /* 最小可分配内存块大小                 */

/* mask with all bits to left of least bit of x on */
#define left_bits(x)                    ((x<<1) | -(x<<1))
#define to_memory_size(ChunkSize)       ((ChunkSize) - sizeof(struct chunk_head))

/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 占用内存块摘要类型 ----------------------------------------------------------------------------------------------*/
struct chunk_head
{
    INT32U              PrevInfo;                                           /* 前块空间信息(块大小 + 状态)          */
    INT32U              ThisInfo;                                           /* 本块空间信息(块大小 + 状态)          */
};

/* 空闲内存块摘要类型 ----------------------------------------------------------------------------------------------*/
struct __chunk
{
    INT32U              PrevInfo;                                           /* 前块空间信息(块大小 + 状态)          */
    INT32U              ThisInfo;                                           /* 本块空间信息(块大小 + 状态)          */
    
    struct __chunk     *pFreeNext;                                          /* 下一空闲块地址                       */
    struct __chunk     *pFreePrev;                                          /* 上一空闲块地址                       */
};
typedef struct __chunk                              CHUNK;                  /* 内存块摘要类型                       */

/* 树型节点块类型 --------------------------------------------------------------------------------------------------*/
struct __tree_node
{
    INT32U              PrevInfo;                                           /* 前块空间信息(块大小 + 状态)          */
    INT32U              ThisInfo;                                           /* 本块空间信息(块大小 + 状态)          */
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

    CHUNK              *pSameChunks[32*2 + 2];                              /* 同型内存容器组                       */
    TNODE              *pTreeChunks[32];                                    /* 树型内存容器组                       */
};
typedef struct __memory_allocor                     MALLOCOR;               /* 内存分配器类型                       */

/*********************************************************************************************************************
                                                  全局变量定义区
*********************************************************************************************************************/
static MALLOCOR             Mallocor;                                       /* 内存分配器                           */
static struct semaphore     AccessLock;                                     /* 访问锁                               */

/*********************************************************************************************************************
** Function name:           get_same_chunk_group
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
INLINE CHUNK * get_same_chunk_group(INT32U Index)
{
    return (CHUNK *)&(Mallocor.pSameChunks[Index << 1]);
}

/*********************************************************************************************************************
** Function name:           splite_free_chunk
** Descriptions:            将一块空闲内存块分割为一块占用内存块和一块空闲内存块. 返回分割后空闲内存块指针.
** Input parameters:        pFreeChunk : 被分割空闲内存块
**                          ChunkSize  : 期望分割内存块大小
** Output parameters:       
** Returned value:          ==NULL : 分割后无空闲块.
**                          !=NULL : 空闲内存块指针.
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-12-5  18:40:8
** Test recorde:            编码->走读->复查->单元测试
**--------------------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Test recorde:            
*********************************************************************************************************************/
CHUNK * splite_free_chunk(CHUNK * pFreeChunk, INT32U ChunkSize)
{
    CHUNK *                 pNextChunk;
    INT32U                  FreeSize;                                       /* 剩余空间大小                         */

    
    FreeSize = pFreeChunk->ThisInfo;
    pNextChunk = (CHUNK *)((char *)pFreeChunk + FreeSize);
    if (FreeSize - ChunkSize < MIN_CHUNK_SIZE)
    {
        pFreeChunk->ThisInfo = pNextChunk->PrevInfo = FreeSize | USED;
        pFreeChunk = NULL;
    }
    else
    {
        CHUNK *             pRestChunk;
        INT32U              RestSize = FreeSize - ChunkSize;
        
        pRestChunk = (CHUNK *)((char *)pFreeChunk + ChunkSize);
        pFreeChunk->ThisInfo = pRestChunk->PrevInfo = ChunkSize | USED;

        pFreeChunk = pRestChunk;
        pFreeChunk->ThisInfo = pNextChunk->PrevInfo = RestSize | FREE;
    }
    
    return pFreeChunk;
}

/*********************************************************************************************************************
** Function name:           combin_free_chunk
** Descriptions:            将两块空闲内存块组合成一块空闲内存块. 返回组合后空闲内存块指针.
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-12-5  18:44:17
** Test recorde:            编码->走读->复查->单元测试
**--------------------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Test recorde:            
*********************************************************************************************************************/
CHUNK * combin_free_chunk(CHUNK * pThisChunk, CHUNK * pNewChunk)
{
    
    return pThisChunk;
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
    return pChunk ? (char *)pChunk + (sizeof(INT32U)*2) : NULL;
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
    pHead = get_same_chunk_group(Index);
    
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
** Function name:           checkin_free_chunk
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
static void checkin_free_chunk(CHUNK *pChunk, INT32U ChunkSize)
{    
    INT32U          Size;


    if (NULL != pChunk)
    {    
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
}

/*********************************************************************************************************************
** Function name:           letfmost_child
** Descriptions:            查找最左侧节点, 如果没有左侧节点, 则返回右侧节点.
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-12-5  21:20:35
** Test recorde:            编码->走读->复查->单元测试
**--------------------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Test recorde:            
*********************************************************************************************************************/
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
** Descriptions:            unlink the free chunk from memory list.
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
** Function name:           checkout_small_chunk_in_group
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
INLINE CHUNK * checkout_small_chunk_in_group(CHUNK *pHead, INT32U Index)
{
    CHUNK               *pNext, *pFirstChunk;

    pFirstChunk = pHead->pFreeNext;
    pNext = pFirstChunk->pFreeNext;
    
    if (pHead == pNext)                                                     /* 仅存在一个空闲chunk                  */
    {
        Mallocor.SameMap &= ~(1ul << Index);
    }

    pHead->pFreeNext = pNext;
    pNext->pFreePrev = pHead;

    return pFirstChunk;
}

/*********************************************************************************************************************
** Function name:           checkout_small_chunk_in_tree
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
static CHUNK* checkout_small_chunk_in_tree(INT32U Size)
{
    INT32U              Index;                                              /* 索引                                 */
    INT32U              FreeSize;                                           /* 最小差额                             */
    TNODE              *pTmp, *pChunk;
    
    
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

    unlink_large_chunk(pChunk);

    return (CHUNK *)pChunk;
}

/*********************************************************************************************************************
** Function name:           checkout_large_chunk_in_tree
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
static CHUNK * checkout_large_chunk_in_tree(INT32U Size) 
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

    unlink_large_chunk(pChunk);
    return (CHUNK *)pChunk; 
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
    int             Counter;
    
    /*
     * 1) 创建管理访问锁
     */
    AccessLock.Counter = 1;
    INIT_LIST_HEAD(&AccessLock.WaitHead);
    
    /*
     * 2) 初始化内存容器组
     */
    memset(&Mallocor, 0, sizeof(Mallocor));                                 /* 清零分配器管理信息                   */
    for (i = 0; i < 32; i++)
    {
        CHUNK *pChunk = get_same_chunk_group(i);
        pChunk->pFreeNext = pChunk;                                         /* 组织环形链表头节点                   */
        pChunk->pFreePrev = pChunk;
    }

    /*
     * 3) 注册所有内存池
     */
    for (Counter = i = 0; i < NumOfHeap; i++)
    {
        INT32U      StartAddr;                                              /* 有效空间起始地址                     */
        INT32U      EndAddr;                                                /* 有效空间结束地址                     */
        INT32U      ChunkSize;                                              /* chunk空间大小                        */
        
        StartAddr = (INT32U)aMemorySegments[i].pStartAddr;
        EndAddr   = (INT32U)aMemorySegments[i].pEndAddr;             

        /* 对齐到ARM系统字对齐格式 */
        StartAddr = align_upside(StartAddr, sizeof(INT32U));
        EndAddr   = align_downside(EndAddr, sizeof(INT32U));
        if (EndAddr < StartAddr)
        {
            DBG_WARN("内存池[%d]分配非法空间\r\n", i);
            continue;
        }
        
        EndAddr  -= sizeof(struct chunk_head);                              /* 减去尾部块标识符空间位置             */
        ChunkSize = EndAddr - StartAddr;
        if (ChunkSize < MIN_CHUNK_SIZE)
        {
            DBG_WARN("内存池[%d]空间小于最小分配单元\r\n", i);
            continue;
        }
        
        /*
         * 3.1) 设定内存块摘要信息
         *   a)标识前一块chunk空间为0字节,且占用.以做为前面边界标识符.
         *   b)标识后一块chunk空间为0字节,且占用.以做为后面边界标识符.
         */
        write_dword(StartAddr, 0ul | USED);                                 /* PrevInfo                             */
        write_dword(StartAddr + sizeof(INT32U), ChunkSize | FREE);          /* ThisInfo                             */        
        write_dword(EndAddr, ChunkSize | FREE);                             /* PrevInfo                             */
        write_dword(EndAddr + sizeof(INT32U), 0ul | USED);                  /* ThisInfo                             */

        /*
         * 3.2) 将空闲内存块插入到分配表
         */
        checkin_free_chunk((CHUNK *)StartAddr, ChunkSize);
        Counter++;
    }

    if (Counter)
        return OK;

    return ERR_NO_MEMERY;
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
    CHUNK           *pChunk;


    DBG_INFO("申请内存空间(size=%d)\r\n", Size);
    
    semaphore_wait(&AccessLock, 0);
    Size = align_upside(Size + MIN_CHUNK_SIZE, sizeof(int));                /* 格式化申请尺寸                       */
    DBG_INFO("对齐内存空间(size=%d)\r\n", Size);

    /*
     * 1) 查找最佳空间匹配内存块,并内存管理器中分离出来.
     */
    if (is_small(Size)) 
    {
        INT32U           Index;                                             /* 内存箱索引号                         */
        INT32U           SameBits;                                          /* 同型内存容器组状态位图               */
        CHUNK           *pHead;
       
        Size       = align_upside(Size, 8);
        Index      = get_box_index(Size);
        SameBits   = Mallocor.SameMap >> Index;                             /* 去除小于不满足的索引位               */
        Index = 0x1F & bit_scan_forward(SameBits << Index);
        if (Index)
        {
            pHead  = get_same_chunk_group(Index);
            pChunk = checkout_small_chunk_in_group(pHead, Index);
        }
        else
        {
            pChunk = checkout_small_chunk_in_tree(Size);
        }
    }
    else
    {
        pChunk = checkout_large_chunk_in_tree(Size);
    }

    /*
     * 2) 初始化申请到的内存块,并将剩余内存块返回给内存管理器.
     *    pChunk: 已申请的内存块;
     *    Size  : 申请空间大小;
     */
    if (NULL != pChunk)
    {
        CHUNK               *pFreeChunk;

        pFreeChunk = splite_free_chunk(pChunk, Size + sizeof(struct chunk_head));

        checkin_free_chunk(pFreeChunk, pFreeChunk->ThisInfo);
    }
    semaphore_post(&AccessLock);
    
    DBG_INFO("内存块信息:\r\nPrevInfo  = 0x%X\r\nThisInfo  = 0x%X\r\npFreeNext = 0x%X\r\npFreePrev = 0x%X\r\n",
             pChunk->PrevInfo, pChunk->ThisInfo, pChunk->pFreeNext, pChunk->pFreePrev);

    return chunk_to_memeory(pChunk);
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

    pThisChunk = (CHUNK *)((int)pMemory - sizeof(struct chunk_head));
    Size = pThisChunk->ThisInfo & ~1ul;
    pNextChunk = (CHUNK *)((int)pThisChunk + Size);

    /*
     * 1) 如果prev内存块为空闲状态, 则将prev内存块和this内存块合并.
     */
    if (FREE == test_bit(pThisChunk->PrevInfo, 0))                          /* 测试上一块是否空闲 ?                 */
    {
        INT32U    PrevSize = pThisChunk->PrevInfo;                          /* prev块大小                           */

        pPrevChunk = (CHUNK *)((int)pThisChunk - PrevSize);                 /* prev内存块                           */
        Size += PrevSize;
        pThisChunk = pPrevChunk;

        unlink_chunk(pPrevChunk, to_memory_size(PrevSize));

        pThisChunk->ThisInfo = pNextChunk->PrevInfo = Size;                
    }

    /*
     * 2) 对next块进行处理
     */
    if (FREE == test_bit(pNextChunk->ThisInfo, 0))                          /* 测试下一块是否空闲 ?                 */
    {
        INT32U    NextSize = pNextChunk->ThisInfo;                          /* next块大小                           */
        CHUNK    *pNextNextChunk = (CHUNK *)((int)pNextChunk + NextSize);
        
        unlink_chunk(pNextChunk, to_memory_size(NextSize));
        Size += NextSize;
        pThisChunk->ThisInfo = pNextNextChunk->PrevInfo = Size;
    }

    /*
     * 3) 将内存块插入列表
     */
    checkin_free_chunk(pThisChunk, Size);
    semaphore_post(&AccessLock);
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

