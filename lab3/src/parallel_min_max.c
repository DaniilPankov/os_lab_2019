#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            // your code here
            // error handling
            break;
          case 1:
            array_size = atoi(optarg);
            // your code here
            // error handling
            break;
          case 2:
            pnum = atoi(optarg);
            // your code here
            // error handling
            break;
          case 3:
            with_files = true;
            break;

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }


  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);

  int active_child_processes = 0;
  
  int chunk_size = array_size /pnum;
  //с использование pipes
  int (*min_pipes)[2] = malloc(sizeof(int[2]) * pnum);
  int (*max_pipes)[2] = malloc(sizeof(int[2]) * pnum);
  for(int i = 0; i < pnum; i++){
    if(pipe(min_pipes[i]) == -1 || pipe(max_pipes[i]) == -1){
      perror("pipe");
      return 1;
    }
  }

  //с использованием файлов
  FILE *min_file;
  FILE *max_file;
  min_file = fopen("min_values.txt", "w");
  fclose(min_file);
  max_file = fopen("max_values.txt", "w");
  fclose(max_file);
  
  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        
        // parallel somehow
        if (with_files) {

          int start_index = i * chunk_size;
          int end_index = (i == pnum - 1) ? array_size : (i+1) * chunk_size;
          struct MinMax local_min_max = GetMinMax(array ,start_index, end_index);
          
          //для минимального значения
          if(is_file_empty(min_file)){
            min_file = fopen("min_values.txt","r+");
            fprintf(min_file,"%d\n", local_min_max.min);
            fclose(min_file);

          }else{
            min_file = fopen("min_values.txt", "r+");
            int min_val_in_file;
            fscanf(min_file, "%d", &min_val_in_file);
            if(local_min_max.min < min_val_in_file){
              fseek(min_file, 0 , SEEK_SET);
              fprintf(min_file, "%d\n", local_min_max.min);
            }
            fclose(min_file);

          }

          //для максимального значения
          if(is_file_empty(max_file)){
            max_file = fopen("max_values.txt","r+");
            fprintf(max_file,"%d\n", local_min_max.max);
            fclose(max_file);

          }else{
            max_file = fopen("max_values.txt", "r+");
            int max_val_in_file;
            fscanf(max_file, "%d", &max_val_in_file);
            if(local_min_max.max > max_val_in_file){
              fseek(max_file, 0 , SEEK_SET);
              fprintf(max_file, "%d\n", local_min_max.max);
            }
            fclose(max_file);

          }

        } else {
          close(min_pipes[i][0]); //закрываем чтение для минимума
          close(max_pipes[i][0]);// закрываем чтение для максимума
          int start_index = i * chunk_size;
          int end_index = (i == pnum - 1) ? array_size : (i+1) * chunk_size;
          struct MinMax local_min_max = GetMinMax(array ,start_index, end_index);
          //printf("local_min: %d local_max: %d\n",local_min_max.min, local_min_max.max);
          write(min_pipes[i][1], &local_min_max.min, sizeof(int));
          write(max_pipes[i][1], &local_min_max.max, sizeof(int));

          close(min_pipes[i][1]); // закрываем запись для минимума
          close(max_pipes[i][1]); // закрываем запись для максимума
            
          // use pipe here
        }
        return 0;
      }
      else{
        close(min_pipes[i][1]); // закрываем запись для минимума
        close(max_pipes[i][1]); // закрываем запись для максимума
      }
    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  while (active_child_processes > 0) {
    // your code here
    int status;
    wait(&status);
    active_child_processes -= 1;
  }
  
  int min_result, max_result;
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      min_file = fopen("min_values.txt", "r");
      max_file = fopen("max_values.txt", "r");
      fscanf(min_file, "%d", &min_result);
      fscanf(max_file, "%d", &max_result);
      fclose(min_file);
      fclose(max_file);
      // read from files
    } else {
      read(min_pipes[i][0], &min_result, sizeof(int));
      read(max_pipes[i][0], &max_result, sizeof(int));
      close(min_pipes[i][0]);//закрываем чтение для минимума
      close(max_pipes[i][0]);//закрываем чтение для максимума
      //free(min_pipes);
      //free(max_pipes);
    }


    if (min_result < min_max.min) min_max.min = min_result;
    if (max_result > min_max.max) min_max.max = max_result;
  }
  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(min_pipes);
  free(max_pipes);
  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}

