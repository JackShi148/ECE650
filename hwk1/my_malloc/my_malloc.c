#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "my_malloc.h"

// global variable
unsigned long data_seg_size = 0;
unsigned long free_space_size = 0;

Metadata * first_free_block = NULL;
Metadata * last_free_block = NULL;
Metadata * first_block = NULL;

void print_free_blocks() {
  printf("from head to tail\n");
  Metadata * cur = first_free_block;
  while(cur != NULL) {
    printf("cur addr: %p, cur->size: %lu\n", cur, cur->size);
    cur= cur->next;
  }
}

void add_free_block(Metadata * ptr) {
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
      last_free_block->next = ptr;
      ptr->prev = last_free_block;
      ptr->next = NULL;
      last_free_block = ptr;
    }
  }
  // cur > ptr
  else {
    // cur is the first free block
    if(cur == first_free_block) {
      first_free_block->prev = ptr;
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

void remove_free_block(Metadata * ptr) {
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

void * allocate_block(size_t size) {
  data_seg_size += size + sizeof(Metadata);
  Metadata * new_block = sbrk(size + sizeof(Metadata));
  if(new_block == (void*)-1) {
    return NULL;
  }
  new_block->available = 0;
  new_block->size = size;
  new_block->next = NULL;
  new_block->prev = NULL;
  return new_block;
}

void * find_ff(size_t size) {
  Metadata * cur_block = first_free_block;
  while(cur_block != NULL && cur_block->size < size) {
    cur_block = cur_block->next;
  }
  if(cur_block == NULL) {
    return NULL;
  }
  return cur_block;
}

void * find_bf(size_t size) {
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

void merge_blocks(Metadata * ptr1, Metadata * ptr2) {
  ptr1->size += ptr2->size + sizeof(Metadata);
  remove_free_block(ptr2);
}

void * ff_malloc(size_t size) {
  if(first_block == NULL || first_free_block == NULL) {
    Metadata * new_block = allocate_block(size);
    // printf("start heap: %p\n", new_block);
    if(new_block == NULL) {
      return NULL;
    }   
    if(first_block == NULL) {
      first_block = new_block;
    }
    return ((char*)new_block + sizeof(Metadata));
  }
  else {
    Metadata * ff_block = find_ff(size);
    if(ff_block == NULL) {
      ff_block = allocate_block(size);
      if(ff_block == NULL) {
        return NULL;
      }
      return ((char*)ff_block + sizeof(Metadata));
    }
    if(ff_block->size > (size + sizeof(Metadata))) {
      ff_block = split_block(ff_block, size);
      free_space_size -= size + sizeof(Metadata);
    }
    else {
      ff_block->available = 0;
      remove_free_block(ff_block);
      free_space_size -= ff_block->size + sizeof(Metadata);
    }
    return ((char*)ff_block + sizeof(Metadata));
  }
}

void ff_free(void * ptr) {
  if(ptr <= (void*)sizeof(Metadata)) {
    return;
  }
  // printf("ptr: %p\n", ptr);
  Metadata * block_to_free = (Metadata*)((char*)ptr - sizeof(Metadata));
  // printf("block_to_free: %p\n", block_to_free);
  free_space_size += block_to_free->size + sizeof(Metadata);
  block_to_free->available = 1;
  add_free_block(block_to_free);
  if(block_to_free->prev != NULL) {
    if(((char*)block_to_free->prev + sizeof(Metadata) + block_to_free->prev->size) == (char*)block_to_free) {
      merge_blocks(block_to_free->prev, block_to_free);
    }
  }
  if(block_to_free->next != NULL) {
      if((char*)block_to_free == ((char*)block_to_free->next - block_to_free->size - sizeof(Metadata))) {
        merge_blocks(block_to_free, block_to_free->next);
    }
  }
}

void * bf_malloc(size_t size) {
  if(first_block == NULL || first_free_block == NULL) {
    Metadata * new_block = allocate_block(size);
    // printf("start heap: %p\n", new_block);
    if(new_block == NULL) {
      return NULL;
    }   
    if(first_block == NULL) {
      first_block = new_block;
    }
    return ((char*)new_block + sizeof(Metadata));
  }
  else {
    Metadata * bf_block = find_bf(size);
    if(bf_block == NULL) {
      bf_block = allocate_block(size);
      if(bf_block == NULL) {
        return NULL;
      }  
      return ((char*)bf_block + sizeof(Metadata));
    }
    // printf("bf_block is: %p, bf_block size is: %lu\n", bf_block, bf_block->size);
    if(bf_block->size > (size + sizeof(Metadata))) {
      bf_block = split_block(bf_block, size);
      free_space_size -= size + sizeof(Metadata);
    }
    else {
      bf_block->available = 0;
      remove_free_block(bf_block);
      free_space_size -= bf_block->size + sizeof(Metadata);
    }
    return ((char*)bf_block + sizeof(Metadata));
  }
}

void bf_free(void * ptr) {
  return ff_free(ptr);
}

unsigned long get_data_segment_size() {
  return data_seg_size;
}

unsigned long get_data_segment_free_space_size() {
  return free_space_size;
}