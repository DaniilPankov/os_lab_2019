#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
void GenerateArray(int *array, unsigned int array_size, unsigned int seed) {
  srand(seed);
  for (int i = 0; i < array_size; i++) {
    array[i] = rand();
  }
}

int is_file_empty(FILE *file){
  file = fopen("min_values.txt", "r+");
  fseek(file, 0, SEEK_END);
  if(ftell(file) == 0){
    fclose(file);
    return 1;
  }
  fclose(file);
  return 0;
}