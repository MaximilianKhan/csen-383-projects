#include <stdio.h>
#include <stdlib.h>
#include "process.h"
#include "utility.h"
#include <time.h>

// Comparison function for sorting processes based on arrival time
int compareProcesses(void *data1, void *data2) {
    process *process1 = (process *)data1;
    process *process2 = (process *)data2;

    if (process1->arrivalTime < process2->arrivalTime) {
        return -1;
    } else if (process1->arrivalTime == process2->arrivalTime) {
        return 0;
    } else {
        return 1;
    }
}

// Function to generate a list of random processes
linked_list *generateProcesses(int numberOfProcesses) {
    linked_list *processList = createLinkedList();
    char processID = 'A';
    unsigned char priority;
    float arrivalTime, runTime;
    time_t t;

    // Initializes random number generator
    srand((unsigned)time(&t));

    if (processList == NULL) {
        fprintf(stderr, "Unable to create Linked List\n");
    }

    while (numberOfProcesses--) {
        arrivalTime = rand() % 100;
        runTime = (float)((rand() % 100) + 1) / 10;
        priority = (rand() % 4) + 1;
        process *newProcess = createProcess(processID, arrivalTime, runTime, priority);

        addNode(processList, newProcess);
        processID++;
    }

    sort(processList, compareProcesses);

    node *pointer = processList->head;
    processID = 'A';

    while (pointer != NULL) {
        ((process *)pointer->data)->pid = processID;
        if (processID == 'Z') {
            processID = 'a' - 1;
        }
        processID++;
        pointer = pointer->next;
    }

    return processList;
}

// Function to create a new process
process *createProcess(char processID, float arrivalTime, float runTime, unsigned char priority) {
    process *newProcess = (process *)malloc(sizeof(process));
    newProcess->pid = processID;
    newProcess->arrivalTime = arrivalTime;
    newProcess->runTime = runTime;
    newProcess->priority = priority;
    return newProcess;
}

// Function to get a copy of a process
// process *getCopyofProcess(process *proc){}


