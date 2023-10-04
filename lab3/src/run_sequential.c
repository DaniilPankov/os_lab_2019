#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Это код, который будет выполняться в дочернем процессе
        char *args[] = {"./sequential_min_max", "123", "1000", NULL};
        execvp(args[0], args);
        perror("execvp");
        return 1;
    } else {
        // Это код родительского процесса
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Child process exited with status %d\n", WEXITSTATUS(status));
        }
    }

    return 0;
}
