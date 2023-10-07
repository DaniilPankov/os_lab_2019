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
#include <signal.h>

#include "find_min_max.h"
#include "utils.h"

pid_t *child_pids;
int pnum = -1;
void handle_alarm(int signo){
  for(int i = 0;i < pnum; i++){
    printf("The child process is killed %d\n", child_pids[i]);
    kill(child_pids[i], SIGKILL);
  }
}

int main(int argc, char **argv) {
  signal(SIGALRM, handle_alarm);
  
  int seed = -1;
  int array_size = -1;
  //int pnum = -1;
  bool with_files = false;
  int timeout = -1;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {"timeout", required_argument, 0, 0},
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
            printf("The pnum is: %d\n",pnum);
            pnum = atoi(optarg);
            printf("The pnum is: %d\n",pnum);
            // your code here
            // error handling
            break;
          case 3:
            with_files = true;
            break;
          case 4:
            printf("The timeout is: %d\n",timeout);
            timeout = atoi(optarg);
            printf("The timeout is: %d\n",timeout);
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
  for(int i = 0; i<array_size;i++){
    printf("Arr[%d]: %d\n",i,array[i]);
  }
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
  
  child_pids = malloc(sizeof(pid_t) * pnum);
  
  struct timeval start_time;
  pid_t parent_id = getpid();
  printf("The parent id is %d\n",parent_id);
  gettimeofday(&start_time, NULL);
  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process
        // parallel somehow
        printf("The child pid is %d \n", getpid());
        if (with_files) {
          // use files here
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
          }
          else{
            max_file = fopen("max_values.txt", "r+");
            int max_val_in_file;
            fscanf(max_file, "%d", &max_val_in_file);
            if(local_min_max.max > max_val_in_file){
              fseek(max_file, 0 , SEEK_SET);
              fprintf(max_file, "%d\n", local_min_max.max);
            }
            fclose(max_file);
          }
        }else{
          // use pipe here
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
            
        }
        return 0;
      }
      else{
        printf("I am parent %d and created child %d\n", getpid(), child_pid);
        child_pids[i] = child_pid;
        close(min_pipes[i][1]); // закрываем запись для минимума
        close(max_pipes[i][1]); // закрываем запись для максимума
      }
    }else{
      printf("Fork failed!\n");
      return 1;
    }
  }

  if(timeout != -1 && getpid() == parent_id){
    printf("Timeout set. Parent process with id %d will send SIGKILL after %d seconds\n", getpid() ,timeout);
    alarm(timeout);
    sleep(timeout);
  }
  int min_result, max_result;
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;
  if (with_files){
    // read from files
    min_file = fopen("min_values.txt", "r");
    max_file = fopen("max_values.txt", "r");
    fscanf(min_file, "%d", &min_result);
    fscanf(max_file, "%d", &max_result);
    fclose(min_file);
    fclose(max_file);
  }else{
    for (int i = 0; i < pnum; i++){
      int min = INT_MAX;
      int max = INT_MIN;
      read(min_pipes[i][0], &min_result, sizeof(int));
      read(max_pipes[i][0], &max_result, sizeof(int));
      close(min_pipes[i][0]);//закрываем чтение для минимума
      close(max_pipes[i][0]);//закрываем чтение для максимума
      if (min_result < min_max.min) min_max.min = min_result;
      if (max_result > min_max.max) min_max.max = max_result;
    }
  }
  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);
  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;
  free(min_pipes);
  free(max_pipes);
  remove("min_values.txt");
  remove("max_values.txt");
  
  free(array);
  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
