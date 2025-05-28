#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>

#define CHILDREN 5
#define DURATION 30
#define BUFFER_SIZE 128
#define READ_END 0
#define WRITE_END 1

// Calculate time difference in seconds
double get_time_diff(struct timeval *start, struct timeval *end) {
    return (end->tv_sec - start->tv_sec) + (end->tv_usec - start->tv_usec) / 1000000.0;
}

// Format time as mm:ss.mmm
void format_time(double time, char *buffer) {
    int minutes = (int)(time / 60);
    int seconds = (int)(time - minutes * 60);
    int milliseconds = (int)((time - (minutes * 60 + seconds)) * 1000);
    sprintf(buffer, "%02d:%02d.%03d", minutes, seconds, milliseconds);
}

int main(void) {
    int fd[CHILDREN][2]; // Pipe file descriptors
    pid_t child_pids[CHILDREN] = {0}; // Child PIDs
    struct timeval start_time, current_time, timeout;
    char buffer[BUFFER_SIZE], time_buf[16];
    fd_set inputs, inputfds;
    int max_fd = 0;

    // Create pipes
    for (int i = 0; i < CHILDREN; i++) {
        if (pipe(fd[i]) == -1) {
            fprintf(stderr, "pipe() failed\n");
            exit(1);
        }
        if (fd[i][READ_END] > max_fd) max_fd = fd[i][READ_END];
    }

    // Get start time
    gettimeofday(&start_time, NULL);

    // Fork child processes
    for (int i = 0; i < CHILDREN; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            fprintf(stderr, "fork() failed\n");
            exit(1);
        } else if (pid == 0) {
            // Child process
            srand(time(NULL) ^ (getpid() << i)); // Unique seed
            for (int j = 0; j < CHILDREN; j++) {
                if (j != i) {
                    close(fd[j][READ_END]);
                    close(fd[j][WRITE_END]);
                }
            }
            close(fd[i][READ_END]); // Close read end

            if (i == 0) {
                // Child 1: Read from stdin
                char input_buf[64];
                int msg_count = 1;
                double elapsed = 0;

                while (elapsed < DURATION) {
                    printf("Enter message for Child 1: ");
                    fflush(stdout);

                    FD_ZERO(&inputs);
                    FD_SET(STDIN_FILENO, &inputs);
                    timeout.tv_sec = DURATION - (int)elapsed;
                    timeout.tv_usec = 0;

                    int result = select(STDIN_FILENO + 1, &inputs, NULL, NULL, &timeout);
                    gettimeofday(&current_time, NULL);
                    elapsed = get_time_diff(&start_time, &current_time);

                    switch (result) {
                        case 0: // Timeout
                            fprintf(stderr, "@");
                            fflush(stderr);
                            break;
                        case -1: // Error
                            perror("select");
                            exit(1);
                        default: // Input available
                            if (FD_ISSET(STDIN_FILENO, &inputs)) {
                                int nread;
                                ioctl(STDIN_FILENO, FIONREAD, &nread);
                                if (nread == 0) {
                                    fprintf(stderr, "Keyboard input done.\n");
                                    break;
                                }
                                nread = read(STDIN_FILENO, input_buf, sizeof(input_buf) - 1);
                                if (nread <= 0) continue;
                                input_buf[nread - 1] = '\0'; // Remove newline

                                format_time(elapsed, time_buf);
                                snprintf(buffer, BUFFER_SIZE, "%s: Child 1 Message %d: %s\n",
                                         time_buf, msg_count++, input_buf);
                                write(fd[i][WRITE_END], buffer, strlen(buffer) + 1);
                            }
                            break;
                    }
                }
            } else {
                // Children 2-5: Generate messages
                int msg_count = 1;
                double elapsed = 0;

                while (elapsed < DURATION) {
                    gettimeofday(&current_time, NULL);
                    elapsed = get_time_diff(&start_time, &current_time);

                    format_time(elapsed, time_buf);
                    snprintf(buffer, BUFFER_SIZE, "%s: Child %d Message %d\n",
                             time_buf, i + 1, msg_count++);
                    write(fd[i][WRITE_END], buffer, strlen(buffer) + 1);

                    int sleep_time = rand() % 3;
                    sleep(sleep_time);
                    elapsed += sleep_time;
                }
            }

            // Send DONE message
            gettimeofday(&current_time, NULL);
            double elapsed = get_time_diff(&start_time, &current_time);
            format_time(elapsed, time_buf);
            snprintf(buffer, BUFFER_SIZE, "%s: Child %d DONE\n", time_buf, i + 1);
            write(fd[i][WRITE_END], buffer, strlen(buffer) + 1);

            close(fd[i][WRITE_END]);
            exit(0);
        } else {
            child_pids[i] = pid;
        }
    }

    // Parent process
    for (int i = 0; i < CHILDREN; i++) {
        close(fd[i][WRITE_END]); // Close write ends
    }

    FILE *out_file = fopen("output.txt", "w");
    if (!out_file) {
        perror("fopen");
        exit(1);
    }

    FD_ZERO(&inputs);
    for (int i = 0; i < CHILDREN; i++) {
        FD_SET(fd[i][READ_END], &inputs);
    }

    int active_children = CHILDREN;
    while (active_children > 0) {
        inputfds = inputs;
        timeout.tv_sec = 2;
        timeout.tv_usec = 500000; // 2.5 seconds

        int result = select(max_fd + 1, &inputfds, NULL, NULL, &timeout);
        switch (result) {
            case 0: // Timeout
                fprintf(stderr, "@");
                fflush(stderr);
                break;
            case -1: // Error
                perror("select");
                exit(1);
            default: // Data available
                for (int i = 0; i < CHILDREN; i++) {
                    if (fd[i][READ_END] != -1 && FD_ISSET(fd[i][READ_END], &inputfds)) {
                        int nread;
                        ioctl(fd[i][READ_END], FIONREAD, &nread);
                        if (nread == 0) {
                            close(fd[i][READ_END]);
                            fd[i][READ_END] = -1;
                            active_children--;
                            continue;
                        }
                        nread = read(fd[i][READ_END], buffer, BUFFER_SIZE - 1);
                        if (nread <= 0) continue;
                        buffer[nread] = '\0';

                        gettimeofday(&current_time, NULL);
                        double parent_time = get_time_diff(&start_time, &current_time);
                        format_time(parent_time, time_buf);
                        fprintf(out_file, "%s: %s", time_buf, buffer);
                        fflush(out_file);
                    }
                }
                break;
        }
    }

    // Wait for all children
    for (int i = 0; i < CHILDREN; i++) {
        waitpid(child_pids[i], NULL, 0);
    }

    fclose(out_file);
    printf("Parent done. Output written to output.txt\n");
    return 0;
}