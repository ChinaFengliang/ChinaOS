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
** Descriptions:         �ڴ����������ʵ���ļ�.
** Hardware platform:    
** SoftWare platform:    ����2.8.4�汾�޸�.
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011��3��20��  18:20:14
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
#define DEBUG_LOCAL_EN                              1                       /* ���ص��Կ���(Ĭ������): 0:��; 1:��   */
/*********************************************************************************************************************
                                                    ͷ�ļ���
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
                                                    �����ʼ�
*********************************************************************************************************************/
/*
 |- 1) �ڴ��
 |       ���ڴ������֧�ֶԶ���ڴ�ؽ��й���.
 |                                           << �ڴ�ؾ��� >>
 |                   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ low
 |       StartAddr ->|                          �ڴ�ؿռ�                           .
 |                   .                                                               .
 |                   .                                                               .
 |                   .                                                               .
 |                   .                                                               .
 |                   .                                                               .
 |                   .                                                               |
 |                   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ high
 |        EndAddr  ->|                         δ֪�ڴ�ռ�                          |
 |                   .                                                               .
 |                   .                                                               .                             
 |                   +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 |       ע: 
 |           �ڴ������: [StartAddr, EndAddr)
 |
 |- 2) �ڴ��
 |       �ڴ���Ƿ������ɷ������СԪ��.
 |                                           << �ڴ�龵�� >>
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
 |       ע: 
 |           b == 0: ����(FREE);
 |           b == 1: ռ��(USED);
 |
 */

/*********************************************************************************************************************
                                                    �궨����
*********************************************************************************************************************/
/* ����� ----------------------------------------------------------------------------------------------------------*/
#define FREE                            0ul                                 /* ����                                 */
#define USED                            1ul                                 /* ռ��                                 */
#define MIN_CHUNK_SIZE                  sizeof(CHUNK)                       /* ��С�ɷ����ڴ���С                 */

/* mask with all bits to left of least bit of x on */
#define left_bits(x)                    ((x<<1) | -(x<<1))
#define to_memory_size(ChunkSize)       ((ChunkSize) - sizeof(struct chunk_head))

/*********************************************************************************************************************
                                                    ���Ͷ�����
*********************************************************************************************************************/
/* ռ���ڴ��ժҪ���� ----------------------------------------------------------------------------------------------*/
struct chunk_head
{
    INT32U              PrevInfo;                                           /* ǰ��ռ���Ϣ(���С + ״̬)          */
    INT32U              ThisInfo;                                           /* ����ռ���Ϣ(���С + ״̬)          */
};

/* �����ڴ��ժҪ���� ----------------------------------------------------------------------------------------------*/
struct __chunk
{
    INT32U              PrevInfo;                                           /* ǰ��ռ���Ϣ(���С + ״̬)          */
    INT32U              ThisInfo;                                           /* ����ռ���Ϣ(���С + ״̬)          */
    
    struct __chunk     *pFreeNext;                                          /* ��һ���п��ַ                       */
    struct __chunk     *pFreePrev;                                          /* ��һ���п��ַ                       */
};
typedef struct __chunk                              CHUNK;                  /* �ڴ��ժҪ����                       */

/* ���ͽڵ������ --------------------------------------------------------------------------------------------------*/
struct __tree_node
{
    INT32U              PrevInfo;                                           /* ǰ��ռ���Ϣ(���С + ״̬)          */
    INT32U              ThisInfo;                                           /* ����ռ���Ϣ(���С + ״̬)          */
    struct __tree_node *pFreeNext;                                          /* ��һ���п��ַ                       */
    struct __tree_node *pFreePrev;                                          /* ��һ���п��ַ                       */

    struct __tree_node *pChild[2];                                          /* ���ֲ��ӽڵ��ַ                     */
    struct __tree_node *pParent;                                            /* ���ڵ��ַ                           */
    INT32U              Index;                                              /* ������                               */
};
typedef struct __tree_node                          TNODE;                  /* ���ڵ�����                           */

/* �ڴ���������� --------------------------------------------------------------------------------------------------*/
struct __memory_allocor
{
    INT32U              SameMap;                                            /* ͬ���ڴ�������״̬λͼ               */
    INT32U              TreeMap;                                            /* �����ڴ�������״̬λͼ               */

    CHUNK              *pSameChunks[32*2 + 2];                              /* ͬ���ڴ�������                       */
    TNODE              *pTreeChunks[32];                                    /* �����ڴ�������                       */
};
typedef struct __memory_allocor                     MALLOCOR;               /* �ڴ����������                       */

/*********************************************************************************************************************
                                                  ȫ�ֱ���������
*********************************************************************************************************************/
static MALLOCOR             Mallocor;                                       /* �ڴ������                           */
static struct semaphore     AccessLock;                                     /* ������                               */

/*********************************************************************************************************************
** Function name:           get_same_chunk_group
** Descriptions:            ��ȡͬ���ڴ�������Ϣ
** Input parameters:        Index : �ڴ���������
** Output parameters:       
** Returned value:          ͷ�ڵ�ָ��
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
** Descriptions:            ��һ������ڴ��ָ�Ϊһ��ռ���ڴ���һ������ڴ��. ���طָ������ڴ��ָ��.
** Input parameters:        pFreeChunk : ���ָ�����ڴ��
**                          ChunkSize  : �����ָ��ڴ���С
** Output parameters:       
** Returned value:          ==NULL : �ָ���޿��п�.
**                          !=NULL : �����ڴ��ָ��.
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-12-5  18:40:8
** Test recorde:            ����->�߶�->����->��Ԫ����
**--------------------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Test recorde:            
*********************************************************************************************************************/
CHUNK * splite_free_chunk(CHUNK * pFreeChunk, INT32U ChunkSize)
{
    CHUNK *                 pNextChunk;
    INT32U                  FreeSize;                                       /* ʣ��ռ��С                         */

    
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
** Descriptions:            ����������ڴ����ϳ�һ������ڴ��. ������Ϻ�����ڴ��ָ��.
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-12-5  18:44:17
** Test recorde:            ����->�߶�->����->��Ԫ����
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
** Descriptions:            ת���ڴ����ַΪӦ���ڴ���ַ
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
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
** Descriptions:            ����box����
** Input parameters:        Size : Ӧ�ÿռ��С(��λ:�ֽ�)
** Output parameters:       
** Returned value:          ��Ӧbox����
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
    return (Size >> 3) - 1;                                                 /* �����ڴ���Ӧ������                 */
}

/*********************************************************************************************************************
** Function name:           tree_size_to_index
** Descriptions:            ��ȡ��Ӧ�ռ��С����Ŀ¼������
** Input parameters:        Size : �ռ��С
** Output parameters:       
** Returned value:          ��Ŀ¼������
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
    else if (x > 0xFFFF)                                                    /* [16777216, �����)                   */
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
** Descriptions:            ��ȡ����������������λ��
** Input parameters:        Index : ��������
** Output parameters:       
** Returned value:          ��Ҫ����λ��
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
** Descriptions:            ��ȡchunk�ռ��С��Ϣ
** Input parameters:        pChunk : �ڴ���׵�ַ
** Output parameters:       
** Returned value:          chunk�ռ��С
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
** Descriptions:            �����ڴ����Ϣ��ͬ�໷�α�
** Input parameters:        pChunk : �ڴ���׵�ַ
**                          Size   : Ӧ���ڴ��С
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
    INT32U              Index;                                              /* ������                               */
    CHUNK              *pHead;                                              /* ͷָ��                               */
    

    Index = get_box_index(Size);    
    pHead = get_same_chunk_group(Index);
    
    if (0 == test_bit(Mallocor.SameMap, Index))
    {
        Mallocor.SameMap |= 1ul << Index;
    }

    /* ��ӵ�����������׽ڵ� */
    pChunk->pFreeNext = pHead->pFreeNext;
    pChunk->pFreePrev = pHead;
    pHead->pFreeNext->pFreePrev = pChunk;
    pHead->pFreeNext  = pChunk;
}

/*********************************************************************************************************************
** Function name:           insert_large_chunk
** Descriptions:            �����ڴ����Ϣ����Ŀ¼
** Input parameters:        pChunk : �ڴ���׵�ַ
**                          Size   : Ӧ���ڴ��С
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
     * A) ����
     */
    if (0ul == test_bit(Mallocor.TreeMap, Index))
    {
        Mallocor.TreeMap |= (1ul << Index); 
        Mallocor.pTreeChunks[Index] = pChunk;
        pChunk->pParent = (TNODE *)&(Mallocor.pTreeChunks[Index]);
        pChunk->pFreeNext = pChunk->pFreePrev = pChunk;                     /* ��������һ��ͬ�ͻ���                 */
        return;
    }

    /*
     * B)�ǿ���
     */
    BitMap = Size << leftshift_for_tree_index(Index);
    ChunkSize = Size + 8;
    while (1)
    {
        if (ChunkSize != get_chunk_size(pNode))
        {
            TNODE **C = &(pNode->pChild[(BitMap >> 31) & 1ul]);
            BitMap <<= 1;

            if (NULL != *C)                                                 /* ��������ӷ�֦ʱ                     */
            {
                /* move into son of the tree */
                pNode = *C;                                                 /* ��������֦                           */
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
** Descriptions:            �ж��Ƿ���С��
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
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
** Input parameters:        pChunk : �ڴ��
**                          ChunkSize : �ڴ���С(�����ײ���Ϣ�ռ�)
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
        Size = ChunkSize - 8;                                                   /* ȥ����ʶ���ռ��С                   */
        
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
** Descriptions:            ���������ڵ�, ���û�����ڵ�, �򷵻��Ҳ�ڵ�.
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-12-5  21:20:35
** Test recorde:            ����->�߶�->����->��Ԫ����
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
** Descriptions:            ɾ���������Ŀ¼�е���Ϣ�ڵ�
** Input parameters:        pChunk : Ŀ������ַ
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
    TNODE               *pReplace;                                          /* ����ڵ�                             */

    
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
** Input parameters:        pHead  : �׽ڵ�
**                          pChunk : Ŀ��ڵ�
**                          Index  : ��������
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
    
    if (pHead == pNext)                                                     /* ������һ������chunk                  */
    {
        Mallocor.SameMap &= ~(1ul << Index);
    }

    pHead->pFreeNext = pNext;
    pNext->pFreePrev = pHead;

    return pFirstChunk;
}

/*********************************************************************************************************************
** Function name:           checkout_small_chunk_in_tree
** Descriptions:            Ѱ����Ŀ¼����С�Ľڵ�
** Input parameters:        Size : �ռ��С(��λ: Byte)
** Output parameters:       
** Returned value:          == NULL : ����ʧ��
**                          != NULL : �ڴ���û�����ַ
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
    INT32U              Index;                                              /* ����                                 */
    INT32U              FreeSize;                                           /* ��С���                             */
    TNODE              *pTmp, *pChunk;
    
    
    if (0 == Mallocor.TreeMap)
    {
        return NULL;
    }

    /*
     * 1) ������С������
     */
    Index  = bit_scan_forward(Mallocor.TreeMap);
    pChunk = pTmp = Mallocor.pTreeChunks[Index];
    FreeSize  = pTmp->ThisInfo;                                             /* ��ΪFREEλ: 0                        */

    /*
     * 2) ������С����Ϣ
     */
    while (NULL != (pTmp = letfmost_child(pTmp)))
    {
        INT32U ThisMargin = pTmp->ThisInfo;                                 /* ���β��                             */

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
** Input parameters:        Size : �ռ��С(��λ: Byte)
** Output parameters:       
** Returned value:          ==NULL : ����ʧ��
**                          !=NULL : �����ɹ�(�ڴ�ռ��ַ)
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
    INT32U       rsize = -Size;                                             /* ��С���                             */
    TNODE       *pChunk = NULL;                                             /* ��ѿ�                               */
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
            INT32U   trem = get_chunk_size(pTree) - Size;                   /* ���β��                             */
            
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
** Descriptions:            ��װ�ڴ������
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
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
     * 1) �������������
     */
    AccessLock.Counter = 1;
    INIT_LIST_HEAD(&AccessLock.WaitHead);
    
    /*
     * 2) ��ʼ���ڴ�������
     */
    memset(&Mallocor, 0, sizeof(Mallocor));                                 /* ���������������Ϣ                   */
    for (i = 0; i < 32; i++)
    {
        CHUNK *pChunk = get_same_chunk_group(i);
        pChunk->pFreeNext = pChunk;                                         /* ��֯��������ͷ�ڵ�                   */
        pChunk->pFreePrev = pChunk;
    }

    /*
     * 3) ע�������ڴ��
     */
    for (Counter = i = 0; i < NumOfHeap; i++)
    {
        INT32U      StartAddr;                                              /* ��Ч�ռ���ʼ��ַ                     */
        INT32U      EndAddr;                                                /* ��Ч�ռ������ַ                     */
        INT32U      ChunkSize;                                              /* chunk�ռ��С                        */
        
        StartAddr = (INT32U)aMemorySegments[i].pStartAddr;
        EndAddr   = (INT32U)aMemorySegments[i].pEndAddr;             

        /* ���뵽ARMϵͳ�ֶ����ʽ */
        StartAddr = align_upside(StartAddr, sizeof(INT32U));
        EndAddr   = align_downside(EndAddr, sizeof(INT32U));
        if (EndAddr < StartAddr)
        {
            DBG_WARN("�ڴ��[%d]����Ƿ��ռ�\r\n", i);
            continue;
        }
        
        EndAddr  -= sizeof(struct chunk_head);                              /* ��ȥβ�����ʶ���ռ�λ��             */
        ChunkSize = EndAddr - StartAddr;
        if (ChunkSize < MIN_CHUNK_SIZE)
        {
            DBG_WARN("�ڴ��[%d]�ռ�С����С���䵥Ԫ\r\n", i);
            continue;
        }
        
        /*
         * 3.1) �趨�ڴ��ժҪ��Ϣ
         *   a)��ʶǰһ��chunk�ռ�Ϊ0�ֽ�,��ռ��.����Ϊǰ��߽��ʶ��.
         *   b)��ʶ��һ��chunk�ռ�Ϊ0�ֽ�,��ռ��.����Ϊ����߽��ʶ��.
         */
        write_dword(StartAddr, 0ul | USED);                                 /* PrevInfo                             */
        write_dword(StartAddr + sizeof(INT32U), ChunkSize | FREE);          /* ThisInfo                             */        
        write_dword(EndAddr, ChunkSize | FREE);                             /* PrevInfo                             */
        write_dword(EndAddr + sizeof(INT32U), 0ul | USED);                  /* ThisInfo                             */

        /*
         * 3.2) �������ڴ����뵽�����
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
** Descriptions:            �����ڴ�ռ�
** Input parameters:        Size : �����ڴ�ռ��С(��λ: Byte)
** Output parameters:       
** Returned value:          ==NULL : ����ʧ��
**                          !=NULL : �����ɹ�(�ڴ�ռ��ַ)
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


    DBG_INFO("�����ڴ�ռ�(size=%d)\r\n", Size);
    
    semaphore_wait(&AccessLock, 0);
    Size = align_upside(Size + MIN_CHUNK_SIZE, sizeof(int));                /* ��ʽ������ߴ�                       */
    DBG_INFO("�����ڴ�ռ�(size=%d)\r\n", Size);

    /*
     * 1) ������ѿռ�ƥ���ڴ��,���ڴ�������з������.
     */
    if (is_small(Size)) 
    {
        INT32U           Index;                                             /* �ڴ���������                         */
        INT32U           SameBits;                                          /* ͬ���ڴ�������״̬λͼ               */
        CHUNK           *pHead;
       
        Size       = align_upside(Size, 8);
        Index      = get_box_index(Size);
        SameBits   = Mallocor.SameMap >> Index;                             /* ȥ��С�ڲ����������λ               */
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
     * 2) ��ʼ�����뵽���ڴ��,����ʣ���ڴ�鷵�ظ��ڴ������.
     *    pChunk: ��������ڴ��;
     *    Size  : ����ռ��С;
     */
    if (NULL != pChunk)
    {
        CHUNK               *pFreeChunk;

        pFreeChunk = splite_free_chunk(pChunk, Size + sizeof(struct chunk_head));

        checkin_free_chunk(pFreeChunk, pFreeChunk->ThisInfo);
    }
    semaphore_post(&AccessLock);
    
    DBG_INFO("�ڴ����Ϣ:\r\nPrevInfo  = 0x%X\r\nThisInfo  = 0x%X\r\npFreeNext = 0x%X\r\npFreePrev = 0x%X\r\n",
             pChunk->PrevInfo, pChunk->ThisInfo, pChunk->pFreeNext, pChunk->pFreePrev);

    return chunk_to_memeory(pChunk);
}

/*********************************************************************************************************************
** Function name:           free
** Descriptions:            �ͷ��ڴ�ռ�
** Input parameters:        pMemory : �ڴ�ռ��ַ
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
    INT32U           Size;                                                  /* ���ڴ���С                         */
    CHUNK           *pThisChunk;                                            /* ��ǰ�ڴ��                           */
    CHUNK           *pNextChunk;                                            /* next�ڴ��                           */
    CHUNK           *pPrevChunk;                                            /* prev�ڴ��                           */

    
    if (NULL == pMemory)
    {
        return;
    }
    
    semaphore_wait(&AccessLock, 0);                                         /* �̻߳��Ᵽ��                         */

    pThisChunk = (CHUNK *)((int)pMemory - sizeof(struct chunk_head));
    Size = pThisChunk->ThisInfo & ~1ul;
    pNextChunk = (CHUNK *)((int)pThisChunk + Size);

    /*
     * 1) ���prev�ڴ��Ϊ����״̬, ��prev�ڴ���this�ڴ��ϲ�.
     */
    if (FREE == test_bit(pThisChunk->PrevInfo, 0))                          /* ������һ���Ƿ���� ?                 */
    {
        INT32U    PrevSize = pThisChunk->PrevInfo;                          /* prev���С                           */

        pPrevChunk = (CHUNK *)((int)pThisChunk - PrevSize);                 /* prev�ڴ��                           */
        Size += PrevSize;
        pThisChunk = pPrevChunk;

        unlink_chunk(pPrevChunk, to_memory_size(PrevSize));

        pThisChunk->ThisInfo = pNextChunk->PrevInfo = Size;                
    }

    /*
     * 2) ��next����д���
     */
    if (FREE == test_bit(pNextChunk->ThisInfo, 0))                          /* ������һ���Ƿ���� ?                 */
    {
        INT32U    NextSize = pNextChunk->ThisInfo;                          /* next���С                           */
        CHUNK    *pNextNextChunk = (CHUNK *)((int)pNextChunk + NextSize);
        
        unlink_chunk(pNextChunk, to_memory_size(NextSize));
        Size += NextSize;
        pThisChunk->ThisInfo = pNextNextChunk->PrevInfo = Size;
    }

    /*
     * 3) ���ڴ������б�
     */
    checkin_free_chunk(pThisChunk, Size);
    semaphore_post(&AccessLock);
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

