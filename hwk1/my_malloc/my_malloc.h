#include <stdio.h>
#include <stdlib.h>

typedef struct metadata_t {
  size_t size;
  int available; // 1 is free, 0 is allocated
  struct metadata_t* next; // next free block
  struct metadata_t* prev; // previous free block
} Metadata;

// first fit algorithm malloc
void * ff_malloc(size_t size);

// first fit algorithm free
void ff_free(void * ptr);

// best fit algorithm malloc
void * bf_malloc(size_t size);

// best fit algorithm free
void bf_free(void * ptr);

// merge blocks pointed by ptr1 and ptr2
void merge_blocks(Metadata * ptr1, Metadata * ptr2);

// find the best fit block
void * find_bf(size_t size);

// find the first fit block
void * find_ff(size_t size);

// split the free block into specific size
void * split_block(Metadata * ptr, size_t size);

// allocate new memory
void * allocate_block(size_t size);

// add ptr into free block
void add_free_block(Metadata * ptr);

// remove ptr from free block
void remove_free_block(Metadata * ptr);

void print_free_blocks();

// return entire heap memory including metadata
unsigned long get_data_segment_size();

// return free block and their metadata
unsigned long get_data_segment_free_space_size();
