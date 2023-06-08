#include <stdio.h>
#include <stdlib.h>

typedef struct metadata_t {
  size_t size;
  int available; // 1 is free, 0 is allocated
  struct metadata_t* next; // next free block
  struct metadata_t* prev; // previous free block
} Metadata;

//Thread Safe malloc/free: locking version
void * ts_malloc_lock(size_t size);

void ts_free_lock(void *ptr);

//Thread Safe malloc/free: non-locking version
void * ts_malloc_nolock(size_t size);

void ts_free_nolock(void *ptr);

// best fit algorithm malloc
void * bf_malloc(size_t size, int sbrk_lock, Metadata * first_free_block, Metadata * last_free_block);

// best fit algorithm free
void bf_free(void * ptr, Metadata * first_free_block, Metadata * last_free_block);

// merge blocks pointed by ptr1 and ptr2
void merge_blocks(Metadata * ptr1, Metadata * ptr2, Metadata * first_free_block, Metadata * last_free_block);

// find the best fit block
void * find_bf(size_t size, Metadata * first_free_block);

// split the free block into specific size
void * split_block(Metadata * ptr, size_t size);

// allocate new memory
void * allocate_block(size_t size, int sbrk_lock);

// add ptr into free block
void add_free_block(Metadata * ptr, Metadata * first_free_block, Metadata * last_free_block);

// remove ptr from free block
void remove_free_block(Metadata * ptr, Metadata * first_free_block, Metadata * last_free_block);

void print_free_blocks();
