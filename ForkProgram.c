#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

/* Ctrl + C handler */
void handle_sigint(int sig) {
    char msg[] = "\n[INTERRUPT] Ctrl+C detected! Program terminated.\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    _exit(0);
}

int main() {
    int n, i, c, fd[2];
    float marks[100], sum = 0;

    signal(SIGINT, handle_sigint);

    /* Parent gets input from keyboard */
    printf("Enter number of students: ");
    scanf("%d", &n);
    if (n < 1 || n > 100) {
        printf("Invalid number of students!\n");
        return 1;
    }

    for (i = 0; i < n; i++) {
        printf("Enter marks for student %d: ", i + 1);
        scanf("%f", &marks[i]);
        if (marks[i] < 0 || marks[i] > 100) {
            printf("Invalid marks! Enter marks from 0 to 100.\n");
            return 1;
        }
        sum += marks[i];
    }

    /* Create 2 children, each with its own pipe */
    for (c = 0; c < 2; c++) {
        pipe(fd);

        if (fork() == 0) {
            /* ---- Child ---- */
            signal(SIGINT, SIG_IGN);
            close(fd[1]);

            int m;
            float data[100];
            read(fd[0], &m, sizeof(m));
            read(fd[0], data, m * sizeof(float));
            close(fd[0]);

            float result = data[0];
            for (i = 1; i < m; i++) {
                if (c == 0 && data[i] > result) result = data[i];
                if (c == 1 && data[i] < result) result = data[i];
            }

            printf("\n--- Child %d (PID: %d) ---\n", c + 1, getpid());
            printf("%s mark: %.2f\n", c == 0 ? "Highest" : "Lowest", result);
            exit(0);
        }

        /* ---- Parent sends message through pipe ---- */
        close(fd[0]);
        write(fd[1], &n, sizeof(n));
        write(fd[1], marks, n * sizeof(float));
        close(fd[1]);
    }

    wait(NULL);
    wait(NULL);

    printf("\n--- Parent (PID: %d) ---\n", getpid());
    printf("Average marks: %.2f\n", sum / n);

    return 0;
}
