#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

#define ALIGNMENT         8        
#define WSIZE             4        
#define DSIZE             8        

#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define MAX(x, y) ((x) > (y)? (x) : (y))
#define PACK(size, alloc) ((size) | (alloc))
#define GET(p)        (*(size_t *)(p))
#define PUT(p, val)   (*(size_t *)(p) = (val))
#define GET_SIZE(p)  (GET(p) & ~0x1)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define HDRP(bp)     ((void *)(bp) - WSIZE)
#define FTRP(bp)     ((void *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((void *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((void *)(bp) - GET_SIZE(HDRP(bp) - WSIZE))
#define NEXT_FREE(bp)(*(void **)(bp))
#define PREV_FREE(bp)(*(void **)(bp + WSIZE))


static void *extend_heap(size_t words);
static void *find_fit(size_t size);
static void *coalesce(void *bp);
static void place(void *bp, size_t asize);
static void remove_freeblock(void *bp);


static char *heap_listp = NULL;
static char *free_listp = NULL;

int mm_init(void)
{
  if ((heap_listp = mem_sbrk(4*DSIZE)) == (void *)-1)
      return -1; 
  PUT(heap_listp,     PACK(2*DSIZE, 1));           
  PUT(heap_listp +    WSIZE,  PACK(2*DSIZE, 0));  
  PUT(heap_listp + (2*WSIZE), PACK(0,0));        
  PUT(heap_listp + (3*WSIZE), PACK(0,0));       
  PUT(heap_listp + (4*WSIZE), PACK(2*DSIZE, 0));
  PUT(heap_listp + (5*WSIZE), PACK(0, 1));     
  free_listp = heap_listp + (WSIZE);
  return 0;
}

void *mm_malloc(size_t size){  
  size_t asize;       
  size_t extendsize; 
  char *bp;
  if (!size)
	  return NULL;
  if (size<=DSIZE)
	  asize = 2*DSIZE;
  else
	  asize = DSIZE*((size+(DSIZE)+(DSIZE-1))/DSIZE);
  if ((bp = find_fit(asize))!=NULL) {
    place(bp, asize);
    return bp;
  }
  extendsize = MAX(asize, 2*DSIZE);
  if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
    return NULL;
  place(bp, asize);
  return bp;
}

void mm_free(void *bp)
{ 
  size_t size = GET_SIZE(HDRP(bp));
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  coalesce(bp);
}

void *mm_realloc(void *bp, size_t size)
{
	void *new;
	size_t oldsize;
	new = mm_malloc(size);
	if (!new)
		return NULL;
	oldsize = GET_SIZE(HDRP(bp));
	if (size < oldsize)
		memcpy(new, bp, size);
	else
		memcpy(new, bp, oldsize);
	mm_free(bp);
	return new;
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; 
	if ((long)(bp = mem_sbrk(size)) == -1) {
        return NULL;
    }
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);   
}

static void *find_fit(size_t size)
{
  void *bp;
  for (bp = free_listp; GET_ALLOC(HDRP(bp)) == 0; bp = NEXT_FREE(bp)) {
    if (size <= GET_SIZE(HDRP(bp))) 
      return bp; 
  }
  return NULL; 
}

static void remove_freeblock(void *bp)
{
  if(bp) {
    if (PREV_FREE(bp))
      NEXT_FREE(PREV_FREE(bp)) = NEXT_FREE(bp);
    else
      free_listp = NEXT_FREE(bp);
    if(NEXT_FREE(bp) != NULL)
      PREV_FREE(NEXT_FREE(bp)) = PREV_FREE(bp);
  }
}

static void *coalesce(void *bp)
{
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))) || PREV_BLKP(bp) == bp;
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));
  if (prev_alloc && !next_alloc) {       
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));  
    remove_freeblock(NEXT_BLKP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
  }
  else if (!prev_alloc && next_alloc) {  
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    bp = PREV_BLKP(bp); 
    remove_freeblock(bp);
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
  } 
  else if (!prev_alloc && !next_alloc) {     
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
            GET_SIZE(HDRP(NEXT_BLKP(bp)));
    remove_freeblock(PREV_BLKP(bp));
    remove_freeblock(NEXT_BLKP(bp));
    bp = PREV_BLKP(bp);
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
  }

  NEXT_FREE(bp) = free_listp;
  PREV_FREE(free_listp) = bp;
  PREV_FREE(bp) = NULL;
  free_listp = bp;

  return bp;
}

static void place(void *bp, size_t asize)
{
  size_t size = GET_SIZE(HDRP(bp));
  size_t remainder = size - asize;
  if((remainder) >= (2*DSIZE)) {
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    remove_freeblock(bp);
    bp = NEXT_BLKP(bp);
    PUT(HDRP(bp), PACK(remainder, 0));
    PUT(FTRP(bp), PACK(remainder, 0));
    coalesce(bp);
  }

  else {

    PUT(HDRP(bp), PACK(size, 1));
    PUT(FTRP(bp), PACK(size, 1));
    remove_freeblock(bp);
  }
}

