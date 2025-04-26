#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stat.h"
#include "process.h"
#include "utility.h"

// Function to create a process_stat structure
process_stat *createProcessStat(process *proc) {
    process_stat *ps = (process_stat *)malloc(sizeof(process_stat));
    ps->proc = proc;
    ps->waitingTime = 0;
    ps->turnaroundTime = 0;
    ps->responseTime = 0;
    ps->startTime = -1;
    ps->endTime = -1;
    ps->runTime = 0;
    ps->state = EMBRYO;
    return ps;
}

// Comparison function for sorting based on runTime
int compareRunTime(void *data1, void *data2) {
    process_stat *ps1 = (process_stat *)data1;
    process_stat *ps2 = (process_stat *)data2;
    if (((process *)ps1->proc)->runTime < ((process *)ps2->proc)->runTime) {
        return -1;
    } else {
        return 1;
    }
}

// Implementation of the Shortest Job First (Non-Preemptive) scheduling algorithm
average_stats shortestJobFirstNP(linked_list *processes) {
    int currentTime = 0;

    // Creation of Process Queue
    queue *processQueue = (queue *)createQueue();
    node *processPtr = processes->head;

    if (processes->head == NULL) {
        fprintf(stderr, "No processes to schedule\n");
    }

    // Keep checking while time quanta is less than 100 or there is a scheduled process
    process_stat *scheduledProcess = NULL;

    linked_list *ll = createLinkedList();
    printf("\nShortest Job First Algorithm:\n");
    printf("Gantt Chart: ");

    while (currentTime < 100 || scheduledProcess != NULL) {
        // Check for incoming new process and enqueue
        if (processPtr != NULL) {
            process *newProcess = (process *)(processPtr->data);
            while (processPtr != NULL && newProcess->arrivalTime <= currentTime) {
                if (newProcess->arrivalTime < 100) {
                    enqueue(processQueue, createProcessStat(newProcess));
                    sort(processQueue, compareRunTime);
                }
                processPtr = processPtr->next;

                if (processPtr != NULL) {
                    newProcess = (process *)(processPtr->data);
                }
            }
        }

        // Check process queue and schedule it if there is no scheduled process now
        if (scheduledProcess == NULL && processQueue->size > 0) {
            scheduledProcess = (process_stat *)dequeue(processQueue);
        }

        if (scheduledProcess != NULL) {
            process *proc = scheduledProcess->proc;

            // Add the currently running process to the time chart
            if (currentTime < 100) {
                printf("%c", proc->pid);
            }

            // Update the current process stat
            if (scheduledProcess->startTime == -1) {
                scheduledProcess->startTime = currentTime;
            }

            scheduledProcess->runTime++;

            if (scheduledProcess->runTime >= proc->runTime) {
                scheduledProcess->endTime = currentTime;
                addNode(ll, scheduledProcess);
                scheduledProcess = NULL;
            }
        } else {
            if (currentTime < 100) {
                printf("_");
            }
        }

        // Keep increasing the time
        currentTime++;
    }

    printf("\n");

    // Printing Process Stat
    return printPolicyStat(ll);
}
