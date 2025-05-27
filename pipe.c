#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 32
#define READ_END    0
#define WRITE_END   1
#define CHILDREN    5
#define DURATION    30

#define INSEC(MIN,SEC,USEC) ((60*MIN)+SEC+(USEC/1000000))

void timeDiff(
        const int *min_start, const int *sec_start, const int *usec_start,
        const int *min_end,   const int *sec_end,   const int *usec_end,
              int *min_diff,        int *sec_diff,        int *usec_diff
        ) {
// CALCULATE THE DIFFERENCE BETWEEN TWO TIME VALUES.
    long start_total = 0, end_total = 0, diff_total = 0;

    start_total += *min_start * 60 * 1000000; 
    start_total += *sec_start * 1000000; 
    start_total += *usec_start; 

    end_total += *min_end * 60 * 1000000; 
    end_total += *sec_end * 1000000; 
    end_total += *usec_end; 

    diff_total = end_total - start_total;

    *min_diff = diff_total / 60 / 1000000;
    diff_total -= *min_diff * 60 * 1000000;

    *sec_diff = diff_total / 1000000;
    diff_total -= *sec_diff * 1000000;

    *usec_diff = diff_total;
    diff_total -= *usec_diff;
}

int main(void) {
// INITIALIZATION.
    int fd[CHILDREN][2] = {0};      // FILE DESCRIPTORS FOR THE PIPES.
    int child_pids[CHILDREN] = {0}; // CHILD PROCESS IDS.
    int pid, child = 0;

    // CREATE THE PIPES.
    for (int i = 0; i < CHILDREN; i++) {
        if (pipe(fd[i]) == -1) {
            fprintf(stderr,"Error: pipe() failed\n");
            return 1;
        }
    }

    // CREATE THE CHILD PROCESSES.
    while (child < CHILDREN) {
        pid = fork();
        if (pid == 0) {                 // CHILD PROCS DO NOT CREATE CHILD PROCS.
            break;
        } else {
            child_pids[child] = pid;    // RECORD CHILD PROC IDS.
            child++;
        }
    }

// INIT TIME.
    struct timeval tv, timeout;
    struct tm *today;
    int min, sec, usec;
    int diff_min, diff_sec, diff_usec;
    int clock = 0, counter = 0, wait_time, result, nread, noresp_count = 0;
    char buffer[128];   // READ BUFFER FOR PIPE.
    fd_set inputfds;    // SET OF FILE DESCRIPTORS BIT-ARRAYS.

    // GET THE PROCESS START TIME.
    gettimeofday(&tv, NULL);
    today = localtime(&tv.tv_sec);
    min = today->tm_min;
    sec = today->tm_sec;
    usec = tv.tv_usec;

// PARENT PROCESS
    if (pid > 0) {
        for (int i = 0; i < CHILDREN; i++)
            close(fd[i][WRITE_END]);    // CLOSE PIPE WRITE ENDS.

        while (noresp_count < 2) {
            timeout.tv_sec = 2;
            timeout.tv_usec = 500000;

            FD_ZERO(&inputfds);         // INITIALIZE INPUTFDS TO THE EMPTY SET.

            for (int i = 0; i < CHILDREN; i++)
                FD_SET(fd[i][READ_END], &inputfds); // ADD CHILD PROC FILE DESCRIPTORS TO SELECT.

            result = select(FD_SETSIZE, &inputfds, (fd_set *) 0, (fd_set *) 0, &timeout);

            switch(result) {
                  case 0: {             // TIMEOUT W/O INPUT.
                    fprintf(stderr, "\r@");
                    fflush(stderr);
                    noresp_count++;
                    break;
                } case -1: {            // ERROR.
                    perror("select()");
                    exit(1);
                } default: {            // READ PIPED DATA FROM CHILD PROCESSES.
                    for (int i = 0; i < CHILDREN; i++) {
                        if (FD_ISSET(fd[i][READ_END], &inputfds)) {
                            ioctl(fd[i][READ_END], FIONREAD, &nread);   // READ # OF BYTES AVAILABLE ON STDIN
                            if (nread == 0) {
                                noresp_count += CHILDREN;
                            } else {
                                nread = read(fd[i][READ_END], buffer, nread);
                                buffer[nread] = '\0';
                                fprintf(stdout, "%s", buffer); fflush(stdout);
                            }
                        }
                    }
                    break;
                }
            }
        }

        for (int i = 0; i < CHILDREN; i++)
            waitpid(child_pids[i], NULL, 0);
        fprintf(stderr, "DONE: Parent\n");
    }
// CHILD PROCESSES: PASS TIMESTAMP
    else if ((pid == 0) && (child != 0)) {        
        srand(time(NULL) + (17*child));
        sleep(rand() % 3);

        close(fd[child][READ_END]);     // CLOSE PIPE READ END.

        while (clock < DURATION) {
            // GET TIME.
            gettimeofday(&tv, NULL);
            today = localtime(&tv.tv_sec);

            // CALC TIMESTAMP.
            timeDiff(
                &min, &sec, &usec,
                &(today->tm_min), &(today->tm_sec), (int*)&(tv.tv_usec),
                &diff_min, &diff_sec, &diff_usec);
            sprintf(buffer, "%02d:%02d.%03d: Child %d Message %d\n",
                    diff_min,
                    diff_sec,
                    diff_usec / 1000,
                    child, counter++);

            // WRITE TO THE PIPE.
            write(fd[child][WRITE_END], buffer, strlen(buffer)+1);

            // INCREMENT CLOCK AND SLEEP.
            wait_time = rand() % 3;
            clock += wait_time;
            sleep(wait_time);
        }

        fprintf(stderr, "DONE: Child %d %05d [%d,%d]\n", child, pid, fd[child][0], fd[child][1]);
        close(fd[child][WRITE_END]);
    }
// CHILD PROCESSES: READ STDIN
    else if ((pid == 0) && (child == 0)) {
        int nread, prev_diff = 0;
        char buf_usr_in[64];

        // fprintf(stderr, "Child %d %05d [%d,%d]\n", child, pid, fd[child][0], fd[child][1]);
        close(fd[child][READ_END]);

        while (clock < DURATION) {
            timeout.tv_sec = DURATION - clock;
            timeout.tv_usec = 00000;

            FD_ZERO(&inputfds);                 // initialize inputfds to the empty set
            FD_SET(STDIN_FILENO, &inputfds);    // set child proc file descriptors

            result = select(STDIN_FILENO+1, &inputfds, (fd_set *) 0, (fd_set *) 0, &timeout);

            // GET TIME.
            gettimeofday(&tv, NULL);
            today = localtime(&tv.tv_sec);

            switch(result) {
                  case 0: {             // TIMEOUT W/O INPUT.
                    fprintf(stderr, "\r#");
                    fflush(stderr);
                    timeDiff(
                        &min, &sec, &usec,
                        &(today->tm_min), &(today->tm_sec), (int*)&(tv.tv_usec),
                        &diff_min, &diff_sec, &diff_usec);
                    break;
                } case -1: {            // ERROR.
                    perror("select()");
                    exit(1);
                } default: {            // READ PIPED DATA FROM CHILD PROCESSES.
                    if (FD_ISSET(STDIN_FILENO, &inputfds)) {
                        ioctl(STDIN_FILENO, FIONREAD, &nread);  // READ # OF BYTES AVAILABLE ON STDIN.
                        if (nread == 0) {
                            fprintf(stderr, "Process done.\n");
                            exit(0);
                        }
                        nread = read(STDIN_FILENO, buf_usr_in, nread);
                        buf_usr_in[nread] = '\0';

                        // CALC TIMESTAMP.
                        timeDiff(
                            &min, &sec, &usec,
                            &(today->tm_min), &(today->tm_sec), (int*)&(tv.tv_usec),
                            &diff_min, &diff_sec, &diff_usec);
                        sprintf(buffer, "%02d:%02d.%03d: Child %d Message %d: %s",
                                diff_min,
                                diff_sec,
                                diff_usec / 1000,
                                child, counter++, buf_usr_in);

                        // WRITE TO THE PIPE.
                        write(fd[child][WRITE_END], buffer, strlen(buffer)+1);
                    }
                }
            }

            // INCREMENT CLOCK AND SLEEP.
            clock += INSEC(diff_min,diff_sec,diff_usec) - prev_diff;
            prev_diff = INSEC(diff_min,diff_sec,diff_usec);
        }

        fprintf(stderr, "DONE: Child %d %05d [%d,%d]\n", child, pid, fd[child][0], fd[child][1]);
        close(fd[child][WRITE_END]);
    } else {
        fprintf(stderr, "Error: fork() failed");
        return 1;
    }
    return 0;
}

