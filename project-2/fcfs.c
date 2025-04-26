#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "stat.h"
#include "process.h"
#include "utility.h"

// Define a local struct that matches your original Process definition
typedef struct {
    int id;
    char name;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;
    int start_time;
    int finish_time;
    int waiting_time;
    int turnaround_time;
    int response_time;
    bool is_completed;
    bool is_started;
} LocalProcess;

// Function prototypes
void run_fcfs(LocalProcess processes[], int num_processes);
void calculate_statistics(LocalProcess processes[], int num_processes, float *avg_response_time, float *avg_wait_time, float *avg_turnaround_time, float *throughput);

// Implementation for the stat.h interface
average_stats firstComeFirstServeNP(linked_list *processes) {
    average_stats stats;
    printf("Running First-Come First-Served (FCFS) algorithm...\n");
    
    // Convert linked_list to array for processing
    LocalProcess proc_array[100];
    int proc_count = 0;
    
    node *current = processes->head;
    while (current != NULL && proc_count < 100) {
        process *p = (process *)current->data;
        proc_array[proc_count].id = proc_count + 1;
        proc_array[proc_count].name = p->pid;
        proc_array[proc_count].arrival_time = p->arrivalTime;
        proc_array[proc_count].burst_time = p->runTime;
        proc_array[proc_count].remaining_time = p->runTime;
        proc_array[proc_count].priority = p->priority;
        proc_array[proc_count].is_completed = false;
        proc_array[proc_count].is_started = false;
        
        current = current->next;
        proc_count++;
    }
    
    // Run FCFS algorithm
    run_fcfs(proc_array, proc_count);
    
    // Calculate statistics
    float avg_response_time, avg_wait_time, avg_turnaround_time, throughput;
    calculate_statistics(proc_array, proc_count, &avg_response_time, &avg_wait_time, &avg_turnaround_time, &throughput);
    
    // Print results
    printf("\nFCFS Results:\n");
    printf("Average Response Time: %.1f\n", avg_response_time);
    printf("Average Wait Time: %.1f\n", avg_wait_time);
    printf("Average Turnaround Time: %.1f\n", avg_turnaround_time);
    printf("Throughput: %.1f processes per 100 quanta\n", throughput);
    
    // Return stats for the main program
    stats.avg_response_time = avg_response_time;
    stats.avg_wait_time = avg_wait_time;
    stats.avg_turnaround = avg_turnaround_time;
    stats.avg_throughput = throughput;
    
    return stats;
}

void run_fcfs(LocalProcess processes[], int num_processes) {
    int current_time = 0;

    // Sort processes by arrival time (should already be sorted)
    for (int i = 0; i < num_processes - 1; i++) {
        for (int j = i + 1; j < num_processes; j++) {
            if (processes[i].arrival_time > processes[j].arrival_time) {
                LocalProcess temp = processes[i];
                processes[i] = processes[j];
                processes[j] = temp;
            }
        }
    }

    printf("Gantt Chart: ");
    
    for (int i = 0; i < num_processes; i++) {
        // Skip to arrival time if needed
        if (current_time < processes[i].arrival_time) {
            for (int j = current_time; j < processes[i].arrival_time; j++) {
                printf("_");
            }
            current_time = processes[i].arrival_time;
        }
        
        // Skip if arrival time is beyond the simulation time
        if (processes[i].arrival_time >= 100) {
            continue;
        }
        
        processes[i].start_time = current_time;
        processes[i].is_started = true;
        processes[i].response_time = processes[i].start_time - processes[i].arrival_time;
        
        // Execute the process for its burst time
        for (int j = 0; j < processes[i].burst_time; j++) {
            if (current_time < 100 || j > 0) {
                printf("%c", processes[i].name);
            }
            current_time++;
        }
        
        processes[i].finish_time = current_time;
        processes[i].is_completed = true;
        processes[i].turnaround_time = processes[i].finish_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
    }
    
    // Fill remaining time with idle
    while (current_time < 100) {
        printf("_");
        current_time++;
    }
    
    printf("\n");
}

void calculate_statistics(LocalProcess processes[], int num_processes, float *avg_response_time, float *avg_wait_time, float *avg_turnaround_time, float *throughput) {
    int total_response_time = 0;
    int total_wait_time = 0;
    int total_turnaround_time = 0;
    int completed_processes = 0;

    for (int i = 0; i < num_processes; i++) {
        if (processes[i].is_started && processes[i].is_completed) {
            total_response_time += processes[i].response_time;
            total_wait_time += processes[i].waiting_time;
            total_turnaround_time += processes[i].turnaround_time;
            if (processes[i].finish_time <= 100) {
                completed_processes++;
            }
        }
    }

    *avg_response_time = (float)total_response_time / num_processes;
    *avg_wait_time = (float)total_wait_time / num_processes;
    *avg_turnaround_time = (float)total_turnaround_time / num_processes;
    *throughput = (float)completed_processes / 100 * 100;
}
