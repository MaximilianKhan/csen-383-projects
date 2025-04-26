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

void run_srt(LocalProcess processes[], int num_processes);
void calculate_stats(LocalProcess processes[], int num_processes, float *avg_rt, float *avg_wt, float *avg_tat, float *throughput);

// Implementation for the stat.h interface
average_stats shortestRemainingTimeP(linked_list *processes) {
    average_stats stats;
    printf("Running Shortest Remaining Time (SRT) algorithm...\n");
    
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
    
    // Run SRT algorithm
    run_srt(proc_array, proc_count);
    
    // Calculate statistics
    float avg_response_time, avg_wait_time, avg_turnaround_time, throughput;
    calculate_stats(proc_array, proc_count, &avg_response_time, &avg_wait_time, &avg_turnaround_time, &throughput);
    
    // Print results
    printf("\nSRT Results:\n");
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

void run_srt(LocalProcess processes[], int num_processes) {
    int time = 0, completed = 0;
    int last_process_id = -1;

    printf("Gantt Chart: ");

    while (completed < num_processes && time < 100) {
        int shortest_idx = -1;
        int min_remaining_time = 9999;

        for (int i = 0; i < num_processes; i++) {
            if (!processes[i].is_completed &&
                processes[i].arrival_time <= time &&
                processes[i].remaining_time < min_remaining_time &&
                processes[i].remaining_time > 0) {
                shortest_idx = i;
                min_remaining_time = processes[i].remaining_time;
            }
        }

        if (shortest_idx == -1) {
            printf("_");  // idle
            time++;
            continue;
        }

        LocalProcess *p = &processes[shortest_idx];

        if (!p->is_started) {
            p->start_time = time;
            p->response_time = p->start_time - p->arrival_time;
            p->is_started = true;
        }

        p->remaining_time--;
        printf("%c", p->name);

        if (p->remaining_time == 0) {
            p->finish_time = time + 1;
            p->turnaround_time = p->finish_time - p->arrival_time;
            p->waiting_time = p->turnaround_time - p->burst_time;
            p->is_completed = true;
            completed++;
        }

        time++;
    }
    
    // Fill remaining time with idle
    while (time < 100) {
        printf("_");
        time++;
    }
    
    printf("\n");
}

void calculate_stats(LocalProcess processes[], int num_processes, float *avg_rt, float *avg_wt, float *avg_tat, float *throughput) {
    int total_rt = 0, total_wt = 0, total_tat = 0, completed = 0;

    for (int i = 0; i < num_processes; i++) {
        if (processes[i].is_started && processes[i].is_completed) {
            total_rt += processes[i].response_time;
            total_wt += processes[i].waiting_time;
            total_tat += processes[i].turnaround_time;
            if (processes[i].finish_time <= 100) {
                completed++;
            }
        }
    }

    *avg_rt = (float)total_rt / num_processes;
    *avg_wt = (float)total_wt / num_processes;
    *avg_tat = (float)total_tat / num_processes;
    *throughput = (float)completed / 100 * 100;
}
