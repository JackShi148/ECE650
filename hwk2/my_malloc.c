#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "my_malloc.h"

// global variable

// head and tail for locked safe malloc
Metadata * first_free_lock_blk = NULL;
Metadata * last_free_lock_blk = NULL;

// head and tail for unsafe malloc in every seperate thread
__thread Metadata * first_free_unlock_blk = NULL;
__thread Metadata * last_free_unlock_blk = NULL;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


// void print_free_blocks() {
//   printf("from head to tail\n");
//   Metadata * cur = first_free_block;
//   while(cur != NULL) {
//     printf("cur addr: %p, cur->size: %lu\n", cur, cur->size);
//     cur= cur->next;
//   }
// }

void * ts_malloc_lock(size_t size) {
  pthread_mutex_lock(&lock);
  int sbrk_lock = 0;
  void * ptr = bf_malloc(size, sbrk_lock, first_free_lock_blk, last_free_lock_blk);
  pthread_mutex_unlock(&lock);
  return ptr;
}

void ts_free_lock(void *ptr) {
  pthread_mutex_lock(&lock);
  bf_free(ptr, first_free_lock_blk, last_free_lock_blk);
  pthread_mutex_unlock(&lock);
}

void * ts_malloc_nolock(size_t size) {
  int sbrk_lock = 1;
  void * ptr = bf_malloc(size, sbrk_lock, first_free_unlock_blk, last_free_unlock_blk);
  return ptr;
}

void ts_free_nolock(void *ptr) {
  bf_free(ptr, first_free_unlock_blk, last_free_unlock_blk);
}

void add_free_block(Metadata * ptr, Metadata * first_free_block, Metadata * last_free_block) {
  Metadata * cur = first_free_block;
  while(cur != NULL && cur < ptr) {
    cur = cur->next;
  }
  // cur == NULL
  if(cur == NULL) {
    if(first_free_block == NULL) {
      first_free_block = ptr;
      last_free_block = ptr;
      ptr->next = NULL;
      ptr->prev = NULL;
    }
    else {
      (last_free_block)->next = ptr;
      ptr->prev = last_free_block;
      ptr->next = NULL;
      last_free_block = ptr;
    }
  }
  // cur > ptr
  else {
    // cur is the first free block
    if(cur == first_free_block) {
      (first_free_block)->prev = ptr;
      ptr->next = first_free_block;
      ptr->prev = NULL;
      first_free_block = ptr;
    }
    else {
      ptr->next = cur;
      ptr->prev = cur->prev;
      cur->prev->next = ptr;
      cur->prev = ptr;
    }
  }
}

void remove_free_block(Metadata * ptr, Metadata * first_free_block, Metadata * last_free_block) {
  // only one block
  if(first_free_block == last_free_block && ptr == first_free_block) {
    first_free_block = last_free_block = NULL;
  }
  // ptr is the first block
  else if(first_free_block == ptr) {
    first_free_block = ptr->next;
    ptr->next->prev = NULL;
  }
  // ptr is the last block
  else if(last_free_block == ptr) {
    last_free_block = ptr->prev;
    ptr->prev->next = NULL;
  }
  // ptr is in the middle
  else {
    ptr->next->prev = ptr->prev;
    ptr->prev->next = ptr->next;
  }
  ptr->prev = NULL;
  ptr->next = NULL;
}

void * split_block(Metadata * ptr, size_t size) {
    ptr->size = ptr->size - size - sizeof(Metadata);
    Metadata * new_block = (Metadata*)((char*)ptr + sizeof(Metadata) + ptr->size);
    new_block->next = NULL;
    new_block->prev = NULL;
    new_block->available = 0;
    new_block->size = size;
    return new_block;
}

void * allocate_block(size_t size, int sbrk_lock) {
  Metadata * new_block = NULL;
  // sbrk_lock == 0: this is thread safe malloc
  if(sbrk_lock == 0) {
    new_block = sbrk(size + sizeof(Metadata));
  }
  // sbrk_lock = 1: this is thread unsafe malloc
  else {
    pthread_mutex_lock(&lock);
    new_block = sbrk(size + sizeof(Metadata));
    pthread_mutex_unlock(&lock);
  }
  if(new_block == (void*)-1) {
    return NULL;
  }
  new_block->available = 0;
  new_block->size = size;
  new_block->next = NULL;
  new_block->prev = NULL;
  return new_block;
}


void * find_bf(size_t size, Metadata * first_free_block) {
  Metadata * cur_block = first_free_block;
  Metadata * bf_block = NULL;
  while(cur_block != NULL) {
    if(cur_block->size >= size) {
      if(bf_block == NULL || bf_block->size > cur_block->size) {
        bf_block = cur_block;
        if(bf_block->size == size) {
          break;
        }
      }
    }
    cur_block = cur_block->next;
  }
  return bf_block;
}

void merge_blocks(Metadata * ptr1, Metadata * ptr2, Metadata * first_free_block, Metadata * last_free_block) {
  ptr1->size += ptr2->size + sizeof(Metadata);
  remove_free_block(ptr2, first_free_block, last_free_block);
}


void * bf_malloc(size_t size, int sbrk_lock, Metadata * first_free_block, Metadata * last_free_block) {
  if(first_free_block == NULL) {
    Metadata * new_block = allocate_block(size, sbrk_lock);
    // printf("start heap: %p\n", new_block);
    if(new_block == NULL) {
      return NULL;
    }
    return ((char*)new_block + sizeof(Metadata));
  }
  else {
    Metadata * bf_block = find_bf(size, first_free_block);
    if(bf_block == NULL) {
      bf_block = allocate_block(size, sbrk_lock);
      if(bf_block == NULL) {
        return NULL;
      }  
      return ((char*)bf_block + sizeof(Metadata));
    }
    // printf("bf_block is: %p, bf_block size is: %lu\n", bf_block, bf_block->size);
    if(bf_block->size > (size + sizeof(Metadata))) {
      bf_block = split_block(bf_block, size);
    }
    else {
      bf_block->available = 0;
      remove_free_block(bf_block, first_free_block, last_free_block);
    }
    return ((char*)bf_block + sizeof(Metadata));
  }
}

void bf_free(void * ptr, Metadata * first_free_block, Metadata * last_free_block) {
  if(ptr <= (void*)sizeof(Metadata)) {
    return;
  }
  // printf("ptr: %p\n", ptr);
  Metadata * block_to_free = (Metadata*)((char*)ptr - sizeof(Metadata));
  // printf("block_to_free: %p\n", block_to_free);
  block_to_free->available = 1;
  add_free_block(block_to_free, first_free_block, last_free_block);
  if(block_to_free->prev != NULL) {
    if(((char*)block_to_free->prev + sizeof(Metadata) + block_to_free->prev->size) == (char*)block_to_free) {
      merge_blocks(block_to_free->prev, block_to_free, first_free_block, last_free_block);
    }
  }
  if(block_to_free->next != NULL) {
      if((char*)block_to_free == ((char*)block_to_free->next - block_to_free->size - sizeof(Metadata))) {
        merge_blocks(block_to_free, block_to_free->next, first_free_block, last_free_block);
    }
  }
}