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
#include    <library/ioport.h>
#include    <library/bit/bit.h>


/*********************************************************************************************************************
                                                    �����ʼ�
*********************************************************************************************************************/
/*
 |- 1) �ڴ��
 |       ���ڴ������֧�ֶԶ���ڴ�ν��й���.
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
 |           �ڴ�����: [StartAddr, EndAddr)
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
#define MIN_CHUNK_SIZE                 16ul                                 /* ��Сchunk�ڴ�ռ��С                */

/* mask with all bits to left of least bit of x on */
#define left_bits(x)         ((x<<1) | -(x<<1))

/*********************************************************************************************************************
                                                    ���Ͷ�����
*********************************************************************************************************************/
/* �ڴ������ ------------------------------------------------------------------------------------------------------*/
struct __segment
{
    void               *pStartAddr;                                         /* �ڴ����ʼ��ַ(����)                 */
    void               *pEndAddr;                                           /* �ڴ�ν�����ַ(������)               */
};
typedef struct __segment                            SEGMENT;                /* �ڴ�ζ�������                       */

/* �ڴ������ ------------------------------------------------------------------------------------------------------*/
struct __chunk
{
    INT32U              PrevInfo;                                           /* ǰ��ռ���Ϣ(��С + ״̬)            */
    INT32U              ThisInfo;                                           /* ����ռ���Ϣ(��С + ״̬)            */
    
    struct __chunk     *pFreeNext;                                          /* ��һ���п��ַ                       */
    struct __chunk     *pFreePrev;                                          /* ��һ���п��ַ                       */
};
typedef struct __chunk                              CHUNK;                  /* �ڴ����Ϣ����                       */

/* �ڴ���������� --------------------------------------------------------------------------------------------------*/
struct __tree_node
{
    INT32U              PrevInfo;                                           /* ǰ��ռ���Ϣ(��С + ״̬)            */
    INT32U              ThisInfo;                                           /* ����ռ���Ϣ(��С + ״̬)            */
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

    CHUNK              *pRecent;                                            /* ���ʹ�ÿ��ַ                       */
    INT32U              SpareSize;                                          /* ʣ��ռ��С(�����С���ʣ��)       */

    CHUNK              *pSameChunks[32*2 + 2];                              /* ͬ���ڴ�������                       */
    TNODE              *pTreeChunks[32];                                    /* �����ڴ�������                       */
};
typedef struct __memory_allocor                     MALLOCOR;               /* �ڴ����������                       */

/*********************************************************************************************************************
                                                  ȫ�ֱ���������
*********************************************************************************************************************/
extern void                *Image$$HeapBase1$$Base;                         /* ����1��ַ                            */
extern void                *Image$$HeapBase2$$Base;                         /* ջ��2��ַ                            */
static MALLOCOR             Mallocor;                                       /* �ڴ������                           */
static struct __semaphore   AccessLock;                                     /* ������                               */
static const SEGMENT        aMemorySegments[] =                             /* �ڴ���б�                           */
{
    {&Image$$HeapBase1$$Base, (void *)0x10008000},
    {&Image$$HeapBase2$$Base, (void *)0x20084000},
};


/*********************************************************************************************************************
** Function name:           heap
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
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
        printk("�ڴ�ռ�%d:[0x%X,0x%X] ��%dK�ֽ�\r\n", i, 
               (INT32U)aMemorySegments[i].pStartAddr, (INT32U)aMemorySegments[i].pEndAddr, 
               (((INT32U)aMemorySegments[i].pEndAddr - (INT32U)aMemorySegments[i].pStartAddr)>>10));
    }
    
    return OK;
}
EXPORT_TO_CONSOLE("�ڴ�ռ�", heap);

/*********************************************************************************************************************
** Function name:           get_chunk
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
INLINE CHUNK * get_chunk(INT32U Index)
{
    return (CHUNK *)&(Mallocor.pSameChunks[Index << 1]);
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
    return (char *)pChunk + (sizeof(INT32U)*2);                             /* �ü���PrevInfo��ThisInfo��ʶ�ռ�     */
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
    pHead = get_chunk(Index);
    
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
** Function name:           insert_chunk
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
static void insert_chunk(CHUNK *pChunk, INT32U ChunkSize)
{    
    INT32U          Size;


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

/*********************************************************************************************************************
** Function name:           update_temporary_chunk
** Descriptions:            ������ʱ��
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
static void* malloc_small_in_tree(INT32U Size)
{
    INT32U              Index;                                              /* ����                                 */
    INT32U              ChunkSize;                                          /* �ڴ��ռ�                           */
    INT32U              FreeSize;                                           /* ��С���                             */
    TNODE              *pTmp, *pChunk, *pNext;
    
    
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

    /*
     * 3) �����С��ռ�
     */
    ChunkSize = pChunk->ThisInfo;
    pNext = (TNODE *)((INT32U)pChunk + ChunkSize);
    unlink_large_chunk(pChunk);
    if (FreeSize - Size < MIN_CHUNK_SIZE)
    {
        /*
         * ��ռ����
         */
        pChunk->ThisInfo = pNext->PrevInfo  = ChunkSize | USED;
    }
    else
    {
        /*
         * ���¶���
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
static void * malloc_large_in_tree(INT32U Size) 
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

    /*
     * ��dv�Ƚ� 
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
INLINE void unlink_first_small_chunk(CHUNK *pHead, CHUNK *pChunk, INT32U Index)
{
    CHUNK               *pNext;

    pNext = pChunk->pFreeNext;
    
    if (pHead == pNext)                                                     /* ������һ������chunk                  */
    {
        Mallocor.SameMap &= ~(1ul << Index);
    }

    pHead->pFreeNext = pNext;
    pNext->pFreePrev = pHead;
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

    /*
     * 1) ���������ź���
     */    
    AccessLock.Counter = 1;
    AccessLock.pThread = NULL;
    
    memset(&Mallocor, 0, sizeof(Mallocor));                                 /* ���������������Ϣ                   */

    /*
     * 2) ��ʼ���ڴ�������
     */
    for (i = 0; i < 32; i++)
    {
        CHUNK *pChunk = get_chunk(i);
        pChunk->pFreeNext = pChunk;                                         /* ��֯��������ͷ�ڵ�                   */
        pChunk->pFreePrev = pChunk;
    }

    /*
     * 3) ע�������ڴ��
     */
    for (i = 0; i < ARRAY_SIZE(aMemorySegments); i++)
    {
        INT32U      StartAddr;                                              /* ��Ч�ռ���ʼ��ַ                     */
        INT32U      EndAddr;                                                /* ��Ч�ռ������ַ                     */
        INT32U      ChunkSize;                                              /* chunk�ռ��С                        */
        
        StartAddr = (INT32U)aMemorySegments[i].pStartAddr;
        EndAddr   = (INT32U)aMemorySegments[i].pEndAddr;             

        /* ���뵽ARMϵͳ�ֶ����ʽ */
        StartAddr = align_upside(StartAddr, sizeof(INT32U));
        EndAddr   = align_downside(EndAddr, sizeof(INT32U));
        
        EndAddr  -= sizeof(INT32U) << 1;                                    /* ��ȥβ�����ʶ���ռ�λ��             */
        ChunkSize = EndAddr - StartAddr;
        
        /*
         * 3.1) �趨�ڴ���ʶ����Ϣ
         *   a)��ʶǰһ��chunk�ռ�Ϊ0�ֽ�,��ռ��.����Ϊǰ��߽��ʶ��.
         *   b)��ʶ��һ��chunk�ռ�Ϊ0�ֽ�,��ռ��.����Ϊ����߽��ʶ��.
         */
        write_dword(StartAddr, 0ul | USED);                                 /* PrevInfo                             */
        write_dword(StartAddr + sizeof(INT32U), ChunkSize | FREE);          /* ThisInfo                             */        
        write_dword(EndAddr, ChunkSize | FREE);                             /* PrevInfo                             */
        write_dword(EndAddr + sizeof(INT32U), 0ul | USED);                  /* ThisInfo                             */

        /*
         * 3.2) ���ڴ����뵽�����
         */
        insert_chunk((CHUNK *)StartAddr, ChunkSize);
    }

    return OK;
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
    void            *pAppMemory;                                            /* Ӧ���ڴ�����ַ                       */
    INT32U           RemainSize;                                            /* �д�ռ��С                         */
    CHUNK           *pChunk;
    CHUNK           *pNext;
     
    semaphore_wait(&AccessLock, 0);
    Size = align_upside(Size + MIN_CHUNK_SIZE, sizeof(int));                /* ��ʽ������ߴ�                       */
    
    if (Size <= 256) 
    {
        INT32U           Index;                                             /* �ڴ���������                         */
        INT32U           SameBits;                                          /* ͬ���ڴ�������״̬λͼ               */
       
        Size       = align_upside(Size, 8);
        Index      = get_box_index(Size);
        SameBits   = Mallocor.SameMap >> Index;                             /* ȥ��С�ڲ����������λ               */
        
        /* 
         * 1) ��������ڴ�ռ�
         *    ���ڽ��������з����ڴ浥Ԫ
         */
        if (0 != (SameBits & 0x03))
        {
            CHUNK           *pHead;
            
            Index  = ~SameBits & 1ul;
            pHead  = get_chunk(Index);                                      /* ��ȡ��������ͷ�ڵ�                   */
            pChunk = pHead->pFreeNext;
            unlink_first_small_chunk(pHead, pChunk, Index);
            /* �޸��ڴ澵�� */
            pNext = (CHUNK *)((char *)pChunk + Size);
            pChunk->ThisInfo = pNext->PrevInfo = Size | USED;

            pAppMemory = chunk_to_memeory(pChunk);
            goto exit;
        }
        
        /*
         * 2) �ֲ������ڴ�ռ�
         *    �ӿ��ٻ������з����ڴ�ռ�,�Դ˱�֤�ֲ��ۺ�ЧӦ.
         */
        if (Size <= Mallocor.SpareSize)
        {
            goto quick;
        }

        /*
         * 3) ��������ڴ�ռ�
         *    �Ӹ����ͬ���ڴ����з����ڴ��.
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
             * ���ڴӵ�1)����䵽����������ٶ��8���ֽ�,�����ٶ��һ��
             * chunk�ռ�, ���Դ˴�ֱ�Ӷ�Ȼ��chunkһ��Ϊ�� 
             */
            pNext = (CHUNK *)((char *)pChunk + Size);
            pChunk->ThisInfo = pNext->PrevInfo = Size | USED;
            pNext->ThisInfo  = RemainSize | FREE;
            update_temporary_chunk(pNext, RemainSize);

            pAppMemory = chunk_to_memeory(pChunk);
            goto exit;
        }

        /*
         * 4) ����Ŀ¼�ṹ�з���С���ڴ�ռ� 
         */
        pAppMemory = malloc_small_in_tree(Size);
        goto exit;
    }

    /*
     * 5) ����Ŀ¼�ṹ�з������ڴ�ռ�
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
     * 2) �ֲ������ڴ�ռ� 
     */    
quick:
    RemainSize = Mallocor.SpareSize - Size;
    pChunk     = Mallocor.pRecent;
    
    if (RemainSize < MIN_CHUNK_SIZE)
    {
        /*
         * �����ü�ʣ����ڴ治���Դ洢��С�ڴ��ʱ, ���˰����е��ڴ��
         * �����ȥ.
         * ע��: �ڴ��ռ�ߴ���SpareSize.
         */
        pNext = (CHUNK *)((char *)pChunk + Mallocor.SpareSize);             /* ��һ���ڴ��                         */
        pChunk->ThisInfo = pNext->PrevInfo = Mallocor.SpareSize | USED;
        Mallocor.SpareSize = 0;
        Mallocor.pRecent = NULL;
    }
    else
    {
        /*
         * �����ü�ʣ����ڴ���ڴ洢��С�ڴ��ʱ, ������Ŀ��ߴ���ڴ��,��
         * ���ü�ʣ����ڴ�鱣���ڿ��ٷ�����.
         * ע��: �ڴ��ռ�ߴ���Size.
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

    pThisChunk = (CHUNK *)((char *)pMemory - MIN_CHUNK_SIZE);
    Size = pThisChunk->ThisInfo & ~1ul;
    pNextChunk = (CHUNK *)((char *)pThisChunk + Size);

    /*
     * 1) ��prev����д���
     */
    if (FREE == test_bit(pThisChunk->PrevInfo, 0))                          /* ������һ���Ƿ���� ?                 */
    {   /* A) prev�����״̬ */
        INT32U    PrevSize = pThisChunk->PrevInfo;                          /* prev���С                           */

        pPrevChunk = (CHUNK *)((char *)pThisChunk - PrevSize);              /* prev�ڴ��                           */
        Size += PrevSize;
        pThisChunk = pPrevChunk;

        if(pThisChunk != Mallocor.pRecent)
        {
            unlink_chunk(pThisChunk, PrevSize);
        }
        else
        {
            if(FREE != test_bit(pNextChunk->ThisInfo, 0))                   /* ���prev�����dv��,��next����ʹ����  */
            {   
                Mallocor.SpareSize = Size;
                pThisChunk->ThisInfo = pPrevChunk->PrevInfo = Size;                
            }
        }
    }

    /*
     * 2) ��next����д���
     */
    if (FREE == test_bit(pNextChunk->ThisInfo, 0))                          /* ������һ���Ƿ���� ?                 */
    {   /* 2.A) next�鴦�ڿ���״̬�� */
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
    {   /* 2.B) next�鴦��ռ��״̬�� */
        pThisChunk->ThisInfo = pNextChunk->PrevInfo = Size;
    }

    /*
     * 3) ���ڴ������б�
     */
    insert_chunk(pThisChunk, Size);
    
exit:
    semaphore_post(&AccessLock);
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

