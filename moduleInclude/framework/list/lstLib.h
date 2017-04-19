#ifndef _LSTLIB_H_
#define _LSTLIB_H_

//#include "vxworks.h"
#ifdef  __cplusplus
extern "C"
{
#endif

typedef struct node 
{
	struct node *next;
	struct node *previous;
} NODE;

typedef struct/* Header for a linked list. */
{
    NODE node;/* Header list node */
    int count;/* Number of nodes in list */
} LIST;

extern void  lstLibInit  (void);
extern void  lstInit     (LIST *pList);
extern void  lstAdd      (LIST *pList, NODE *pNode);
extern void  lstConcat   (LIST *pDstList, LIST *pAddList);
extern int   lstCount    (LIST *pList);
extern void  lstDelete   (LIST *pList, NODE *pNode);
extern void  lstExtract  (LIST *pSrcList, NODE *pStartNode, NODE *pEndNode, LIST *pDstList);
extern NODE *lstFirst    (LIST *pList);
extern NODE *lstGet      (LIST *pList);
extern void  lstInsert   (LIST *pList, NODE *pPrev, NODE *pNode);
extern NODE *lstLast     (LIST *pList);
extern NODE *lstNext     (NODE *pNode);
extern NODE *lstNth      (LIST *pList, int nodenum);
extern NODE *lstPrevious (NODE *pNode);
extern NODE *lstNStep    (NODE *pNode, int nStep);
extern int   lstFind     (LIST *pList, NODE *pNode);
extern void  lstFree     (LIST *pList);

#ifdef  __cplusplus
}
#endif

#endif /* LSTLIB_H_ */
