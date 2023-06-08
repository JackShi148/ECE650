#include <stdlib.h>
#include <stdio.h>
#include "my_malloc.h"

#ifdef FF
#define MALLOC(sz) ff_malloc(sz)
#define FREE(p)    ff_free(p)
#endif
#ifdef BF
#define MALLOC(sz) bf_malloc(sz)
#define FREE(p)    bf_free(p)
#endif


int main(int argc, char *argv[])
{
  const unsigned NUM_ITEMS = 10;
  int i;
  int size;
  int sum = 0;
  int expected_sum = 0;
  int *array[NUM_ITEMS];

  size = 4;
  printf("size: %d\n", size);
  expected_sum += size * size;
  array[0] = (int *)MALLOC(size * sizeof(int));
  // printf("array[0]: %p\n", array[0]);
  for (i=0; i < size; i++) {
    array[0][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[0][i];
  } //for i
  print_free_blocks();

  size = 16;
  printf("size: %d\n", size);
  expected_sum += size * size;
  array[1] = (int *)MALLOC(size * sizeof(int));
  // printf("array[1]: %p\n", array[1]);
  for (i=0; i < size; i++) {
    array[1][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[1][i];
  } //for i
  print_free_blocks();

  size = 8;
  printf("size: %d\n", size);
  expected_sum += size * size;
  array[2] = (int *)MALLOC(size * sizeof(int));
  // printf("array[2]: %p\n", array[2]);
  for (i=0; i < size; i++) {
    array[2][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[2][i];
  } //for i
  print_free_blocks();

  size = 32;
  printf("size: %d\n", size);
  expected_sum += size * size;
  array[3] = (int *)MALLOC(size * sizeof(int));
  // printf("array[3]: %p\n", array[3]);
  for (i=0; i < size; i++) {
    array[3][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[3][i];
  } //for i
  print_free_blocks();
   FREE(array[0]);
   FREE(array[2]);


  size = 7;
  printf("size: %d\n", size);
  expected_sum += size * size;
  array[4] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[4][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[4][i];
  } //for i
  print_free_blocks();

  size = 256;
  printf("size: %d\n", size);
  expected_sum += size * size;
  array[5] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[5][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[5][i];
  } //for i
  print_free_blocks();

  FREE(array[5]);
  FREE(array[1]);
  FREE(array[3]);

  size = 23;
  printf("size: %d\n", size);
  expected_sum += size * size;
  array[6] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[6][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[6][i];
  } //for i
  print_free_blocks();

  size = 4;
  printf("size: %d\n", size);
  expected_sum += size * size;
  array[7] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[7][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[7][i];
  } //for i
  print_free_blocks();

  FREE(array[4]);
  // print_free_blocks();

  size = 10;
  printf("size: %d\n", size);
  expected_sum += size * size;
  array[8] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[8][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[8][i];
  } //for i
  print_free_blocks();

  size = 32;
  printf("size: %d\n", size);
  expected_sum += size * size;
  array[9] = (int *)MALLOC(size * sizeof(int));
  for (i=0; i < size; i++) {
    array[9][i] = size;
  } //for i
  for (i=0; i < size; i++) {
    sum += array[9][i];
  } //for i
  print_free_blocks();

  FREE(array[6]);
  FREE(array[7]);
  FREE(array[8]);
  FREE(array[9]);
  print_free_blocks();
  printf("data_segment_size: %lu\n", get_data_segment_size());
  printf("data_segment_free_space_size: %lu\n",get_data_segment_free_space_size());

  if (sum == expected_sum) {
    printf("Calculated expected value of %d\n", sum);
    printf("Test passed\n");
  } else {
    printf("Expected sum=%d but calculated %d\n", expected_sum, sum);
    printf("Test failed\n");
  } //else

  return 0;
 }
