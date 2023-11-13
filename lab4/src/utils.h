#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>

struct MinMax {
  int min;
  int max;
};

void GenerateArray(int *array, unsigned int array_size, unsigned int seed);
int is_file_empty(FILE *file);
#endif
