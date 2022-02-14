//------------------------------------------------------------------------------
//
// memtrace
//
// trace calls to the dynamic memory manager
//
#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memlog.h>
#include <memlist.h>
#include <stdbool.h>
//
// function pointers to stdlib's memory management functions
//
static void *(*mallocp)(size_t size) = NULL;
static void (*freep)(void *ptr) = NULL;
static void *(*callocp)(size_t nmemb, size_t size);
static void *(*reallocp)(void *ptr, size_t size);

//
// statistics & other global variables
//
static unsigned long n_malloc  = 0;
static unsigned long n_calloc  = 0;
static unsigned long n_realloc = 0;
static unsigned long n_allocb  = 0;
static unsigned long n_freeb   = 0;
static item *list = NULL;

//
// init - this function is called once when the shared library is loaded
//
__attribute__((constructor))
void init(void)
{
  char *error;

  LOG_START();

  // initialize a new list to keep track of all memory (de-)allocations
  // (not needed for part 1)
  list = new_list();

  // ...
}

//
// fini - this function is called once when the shared library is unloaded
//
__attribute__((destructor))
void fini(void)
{
  // ...

  LOG_STATISTICS(n_allocb, n_allocb/(n_malloc + n_calloc + n_realloc), n_freeb);
  bool flag = false;
  
  if (list){
	item *iter = list->next;
	while (iter){
		if (iter->cnt > 0){
			if (!flag){
				LOG_NONFREED_START();
				flag = true;
			}
			LOG_BLOCK(iter->ptr, iter->size, iter->cnt);
		}
		iter = iter->next;
	}
  }
  LOG_STOP();

  // free list (not needed for part 1)
  free_list(list);
}

// ...

//Implementation Starts Here!

void *malloc(size_t size){
	void *(*mallocp)(size_t size);
	n_allocb += size;
	n_malloc++;
	char *error;
	mallocp = dlsym(RTLD_NEXT, "malloc");

	if ((error = dlerror())!=NULL){
		fputs(error, stderr);
		exit(-1);
	}
	char *ptr = mallocp(size);
	alloc(list, ptr, size);
	LOG_MALLOC((int)size, ptr);
	return ptr;
}

void *calloc(size_t nmemb, size_t size){
	void *(*callocp)(size_t nmemb, size_t size);
	n_allocb += nmemb*size;
	n_calloc++;
	char *error;
	callocp = dlsym(RTLD_NEXT, "calloc");
	if ((error = dlerror())!=NULL){
		fputs(error, stderr);
		exit(-1);
	}
	char *ptr = callocp(nmemb, size);
	alloc(list, ptr, size*nmemb);
	LOG_CALLOC(nmemb, size, ptr);
	return ptr;
}

void *realloc(void *ptr, size_t size){
	void *(*realloc)(size_t size);
	n_realloc++;
	n_allocb+=size;
	char *error;
	void* relocptr = NULL;
	reallocp = dlsym(RTLD_NEXT, "realloc");
	if ((error = dlerror())!= NULL){
		fputs(error, stderr);
		exit(-1);
	}
	item* blk =  find(list, ptr);
	if (!blk){
		LOG_ILL_FREE();
	}
	else if (blk->cnt<=0){
		LOG_DOUBLE_FREE();
	}
	else{
		if (blk->size > size)
			n_freeb += (blk->size)-size;
		dealloc(list, ptr);
		relocptr = reallocp(ptr, size);
		alloc(list, relocptr, size);
		LOG_REALLOC(ptr, size, relocptr);
	}
	return relocptr;
}

void free(void *ptr){
	void (*freep)(void*) = NULL;
	char *error;
	if (!ptr) return;
	freep = dlsym(RTLD_NEXT, "free");
	if ((error = dlerror()) != NULL){
		fputs(error, stderr);
		exit(1);
	}
	LOG_FREE(ptr);
	item *freeptr = find(list, ptr);
	if (!freeptr){
		LOG_ILL_FREE();
	}
	else if (freeptr->cnt<=0){
		LOG_DOUBLE_FREE();
	}
	else{
		freep(ptr);
		n_freeb+=dealloc(list, ptr)->size;
	}
}
