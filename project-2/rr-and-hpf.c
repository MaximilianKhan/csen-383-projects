#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "stat.h"
#include "process.h"
#include "utility.h"

#define MAX_QUANTA 100
#define PRIORITY_LEVELS 4
#define AGING_THRESHOLD 5 // For extra credit: aging after 5 quanta

// Structure declarations for local use
typedef struct {
    char name;           // Process name (A, B, C, ...)
    int arrival_time;    // Arrival time (0–99)
    int run_time;        // Expected run time (1–10)
    int priority;        // Priority (1–4, 1 is highest)
    int remaining_time;  // Remaining run time
    int start_time;      // First CPU allocation time
    int end_time;        // Completion time
    int wait_time;       // Total waiting time
    int response_time;   // Time from arrival to first CPU
    int waiting_since;   // For aging: time since last priority bump
    int ran;             // Flag: 1 if process started execution
} RRProcess;

typedef struct {
    RRProcess* processes[100];
    int front, rear, count;
} RRQueue;

typedef struct {
    double avg_turnaround;
    double avg_waiting;
    double avg_response;
    double throughput;
} RRStats;

// Function declarations
void init_queue(RRQueue* q);
void rr_enqueue(RRQueue* q, RRProcess* p);
RRProcess* rr_dequeue(RRQueue* q);
void generate_rr_processes(RRProcess processes[], int count, int seed);
RRStats round_robin(RRProcess processes[], int process_count, int run, int time_slice);
RRStats hpf(RRProcess processes[], int process_count, int run, int preemptive, int use_aging);

// Convert our stats to the project's stat interface
average_stats convert_stats(RRStats stats) {
    average_stats result;
    result.avg_response_time = (float)stats.avg_response;
    result.avg_wait_time = (float)stats.avg_waiting;
    result.avg_turnaround = (float)stats.avg_turnaround;
    result.avg_throughput = (float)stats.throughput;
    return result;
}

// Round Robin algorithm implementation for the stat.h interface
average_stats roundRobinP(linked_list *processes, int time_slice) {
    printf("Running Round Robin (RR) algorithm with time slice %d...\n", time_slice);
    
    // Convert linked_list to RRProcess array
    RRProcess proc_array[100];
    int proc_count = 0;
    
    node *current = processes->head;
    while (current != NULL && proc_count < 100) {
        process *p = (process *)current->data;
        proc_array[proc_count].name = p->pid;
        proc_array[proc_count].arrival_time = p->arrivalTime;
        proc_array[proc_count].run_time = p->runTime;
        proc_array[proc_count].priority = p->priority;
        proc_array[proc_count].remaining_time = p->runTime;
        proc_array[proc_count].start_time = -1;
        proc_array[proc_count].end_time = -1;
        proc_array[proc_count].wait_time = 0;
        proc_array[proc_count].response_time = -1;
        proc_array[proc_count].waiting_since = proc_array[proc_count].arrival_time;
        proc_array[proc_count].ran = 0;
        
        current = current->next;
        proc_count++;
    }
    
    // Run algorithm and get stats
    RRStats stats = round_robin(proc_array, proc_count, 0, time_slice);
    
    // Convert to average_stats format and return
    return convert_stats(stats);
}

// HPF Preemptive algorithm implementation
average_stats highestPriorityFirstP(linked_list *processes) {
    printf("Running Highest Priority First (HPF) Preemptive algorithm...\n");
    
    // Convert linked_list to RRProcess array
    RRProcess proc_array[100];
    int proc_count = 0;
    
    node *current = processes->head;
    while (current != NULL && proc_count < 100) {
        process *p = (process *)current->data;
        proc_array[proc_count].name = p->pid;
        proc_array[proc_count].arrival_time = p->arrivalTime;
        proc_array[proc_count].run_time = p->runTime;
        proc_array[proc_count].priority = p->priority;
        proc_array[proc_count].remaining_time = p->runTime;
        proc_array[proc_count].start_time = -1;
        proc_array[proc_count].end_time = -1;
        proc_array[proc_count].wait_time = 0;
        proc_array[proc_count].response_time = -1;
        proc_array[proc_count].waiting_since = proc_array[proc_count].arrival_time;
        proc_array[proc_count].ran = 0;
        
        current = current->next;
        proc_count++;
    }
    
    // Run algorithm and get stats (1 = preemptive, 0 = no aging)
    RRStats stats = hpf(proc_array, proc_count, 0, 1, 0);
    
    // Convert to average_stats format and return
    return convert_stats(stats);
}

// HPF Non-Preemptive algorithm implementation
average_stats highestPriorityFirstNP(linked_list *processes) {
    printf("Running Highest Priority First (HPF) Non-Preemptive algorithm...\n");
    
    // Convert linked_list to RRProcess array
    RRProcess proc_array[100];
    int proc_count = 0;
    
    node *current = processes->head;
    while (current != NULL && proc_count < 100) {
        process *p = (process *)current->data;
        proc_array[proc_count].name = p->pid;
        proc_array[proc_count].arrival_time = p->arrivalTime;
        proc_array[proc_count].run_time = p->runTime;
        proc_array[proc_count].priority = p->priority;
        proc_array[proc_count].remaining_time = p->runTime;
        proc_array[proc_count].start_time = -1;
        proc_array[proc_count].end_time = -1;
        proc_array[proc_count].wait_time = 0;
        proc_array[proc_count].response_time = -1;
        proc_array[proc_count].waiting_since = proc_array[proc_count].arrival_time;
        proc_array[proc_count].ran = 0;
        
        current = current->next;
        proc_count++;
    }
    
    // Run algorithm and get stats (0 = non-preemptive, 0 = no aging)
    RRStats stats = hpf(proc_array, proc_count, 0, 0, 0);
    
    // Convert to average_stats format and return
    return convert_stats(stats);
}

// HPF Preemptive with Aging algorithm implementation for the extra credit
average_stats highest_priority_first_p_aging(linked_list *processes) {
    printf("Running Highest Priority First (HPF) Preemptive with Aging algorithm...\n");
    
    // Convert linked_list to RRProcess array
    RRProcess proc_array[100];
    int proc_count = 0;
    
    node *current = processes->head;
    while (current != NULL && proc_count < 100) {
        process *p = (process *)current->data;
        proc_array[proc_count].name = p->pid;
        proc_array[proc_count].arrival_time = p->arrivalTime;
        proc_array[proc_count].run_time = p->runTime;
        proc_array[proc_count].priority = p->priority;
        proc_array[proc_count].remaining_time = p->runTime;
        proc_array[proc_count].start_time = -1;
        proc_array[proc_count].end_time = -1;
        proc_array[proc_count].wait_time = 0;
        proc_array[proc_count].response_time = -1;
        proc_array[proc_count].waiting_since = proc_array[proc_count].arrival_time;
        proc_array[proc_count].ran = 0;
        
        current = current->next;
        proc_count++;
    }
    
    // Run algorithm and get stats (1 = preemptive, 1 = use aging)
    RRStats stats = hpf(proc_array, proc_count, 0, 1, 1);
    
    // Convert to average_stats format and return
    return convert_stats(stats);
}

// HPF Non-Preemptive with Aging algorithm implementation for the extra credit
average_stats highest_priority_first_np_aging(linked_list *processes) {
    printf("Running Highest Priority First (HPF) Non-Preemptive with Aging algorithm...\n");
    
    // Convert linked_list to RRProcess array
    RRProcess proc_array[100];
    int proc_count = 0;
    
    node *current = processes->head;
    while (current != NULL && proc_count < 100) {
        process *p = (process *)current->data;
        proc_array[proc_count].name = p->pid;
        proc_array[proc_count].arrival_time = p->arrivalTime;
        proc_array[proc_count].run_time = p->runTime;
        proc_array[proc_count].priority = p->priority;
        proc_array[proc_count].remaining_time = p->runTime;
        proc_array[proc_count].start_time = -1;
        proc_array[proc_count].end_time = -1;
        proc_array[proc_count].wait_time = 0;
        proc_array[proc_count].response_time = -1;
        proc_array[proc_count].waiting_since = proc_array[proc_count].arrival_time;
        proc_array[proc_count].ran = 0;
        
        current = current->next;
        proc_count++;
    }
    
    // Run algorithm and get stats (0 = non-preemptive, 1 = use aging)
    RRStats stats = hpf(proc_array, proc_count, 0, 0, 1);
    
    // Convert to average_stats format and return
    return convert_stats(stats);
}

// --- Original functions from rr-and-hpf.c below ---

void init_queue(RRQueue* q) {
    q->front = 0;
    q->rear = -1;
    q->count = 0;
}

void rr_enqueue(RRQueue* q, RRProcess* p) {
    if (q->count < 100) {
        q->rear = (q->rear + 1) % 100;
        q->processes[q->rear] = p;
        q->count++;
    }
}

RRProcess* rr_dequeue(RRQueue* q) {
    if (q->count == 0) return NULL;
    RRProcess* p = q->processes[q->front];
    q->front = (q->front + 1) % 100;
    q->count--;
    return p;
}

// Generate random processes
void generate_rr_processes(RRProcess processes[], int count, int seed) {
    srand(seed);
    for (int i = 0; i < count; i++) {
        processes[i].name = 'A' + i;
        processes[i].arrival_time = rand() % 100; // 0–99
        processes[i].run_time = (rand() % 10) + 1; // 1–10
        processes[i].priority = (rand() % 4) + 1; // 1–4
        processes[i].remaining_time = processes[i].run_time;
        processes[i].start_time = -1;
        processes[i].end_time = -1;
        processes[i].wait_time = 0;
        processes[i].response_time = -1;
        processes[i].waiting_since = processes[i].arrival_time;
        processes[i].ran = 0;
    }

    // Sort by arrival time
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (processes[i].arrival_time > processes[j].arrival_time) {
                RRProcess temp = processes[i];
                processes[i] = processes[j];
                processes[j] = temp;
            }
        }
    }
}

RRStats round_robin(RRProcess processes[], int process_count, int run, int time_slice) {
    RRQueue ready_queue;
    init_queue(&ready_queue);
    int current_time = 0;
    int process_index = 0;
    int completed = 0;
    char timeline[MAX_QUANTA + 50]; // Timeline for 100+ quanta
    
    // Initialize timeline
    for (int i = 0; i < MAX_QUANTA + 50; i++) {
        timeline[i] = '-';
    }

    printf("Gantt Chart: ");
    
    while (completed < process_count && current_time < MAX_QUANTA + 50) {
        // Add arrived processes to queue
        while (process_index < process_count && processes[process_index].arrival_time <= current_time) {
            if (processes[process_index].arrival_time <= MAX_QUANTA) {
                rr_enqueue(&ready_queue, &processes[process_index]);
            }
            process_index++;
        }

        // Execute process for time_slice or until completion
        RRProcess* current = rr_dequeue(&ready_queue);
        if (current) {
            if (current->start_time == -1) {
                current->start_time = current_time;
                current->response_time = current_time - current->arrival_time;
                current->ran = 1;
            }
            
            int execute_time = 0;
            while (execute_time < time_slice && current->remaining_time > 0) {
                if (current_time < MAX_QUANTA) {
                    printf("%c", current->name);
                    timeline[current_time] = current->name;
                }
                current->remaining_time--;
                execute_time++;
                current_time++;
                
                // Check for new arrivals during execution
                while (process_index < process_count && processes[process_index].arrival_time <= current_time) {
                    if (processes[process_index].arrival_time <= MAX_QUANTA) {
                        rr_enqueue(&ready_queue, &processes[process_index]);
                    }
                    process_index++;
                }
            }
            
            if (current->remaining_time > 0) {
                rr_enqueue(&ready_queue, current); // Requeue if not finished
            } else {
                current->end_time = current_time;
                completed++;
            }
            
            // Update wait times for queued processes
            for (int i = 0; i < ready_queue.count; i++) {
                int idx = (ready_queue.front + i) % 100;
                ready_queue.processes[idx]->wait_time += execute_time;
            }
        } else {
            if (current_time < MAX_QUANTA) {
                printf("_");
                timeline[current_time] = '-'; // CPU idle
            }
            current_time++;
        }
    }

    printf("\n");
    
    double total_turnaround = 0, total_waiting = 0, total_response = 0;
    int ran_count = 0;
    for (int i = 0; i < process_count; i++) {
        if (processes[i].ran) {
            if (processes[i].end_time > 0) { // Verify end_time is valid
                double turnaround = processes[i].end_time - processes[i].arrival_time;
                total_turnaround += turnaround;
                total_waiting += processes[i].wait_time;
                total_response += processes[i].response_time;
                ran_count++;
            }
        }
    }
    
    RRStats stats = {
        ran_count ? total_turnaround / ran_count : 0,
        ran_count ? total_waiting / ran_count : 0,
        ran_count ? total_response / ran_count : 0,
        ran_count ? (double)ran_count / MAX_QUANTA * 100 : 0  // Fix throughput calculation
    };

    return stats;
}

RRStats hpf(RRProcess processes[], int process_count, int run, int preemptive, int use_aging) {
    RRQueue priority_queues[PRIORITY_LEVELS];
    for (int i = 0; i < PRIORITY_LEVELS; i++) {
        init_queue(&priority_queues[i]);
    }
    
    int current_time = 0;
    int process_index = 0;
    int completed = 0;
    char timeline[MAX_QUANTA + 50]; // Timeline for 100+ quanta
    
    // Initialize timeline
    for (int i = 0; i < MAX_QUANTA + 50; i++) {
        timeline[i] = '-';
    }

    printf("Gantt Chart: ");
    
    while (completed < process_count && current_time < MAX_QUANTA + 50) {
        // Add arrived processes to their priority queues
        while (process_index < process_count && processes[process_index].arrival_time <= current_time) {
            if (processes[process_index].arrival_time <= MAX_QUANTA) {
                int pri = processes[process_index].priority - 1;
                rr_enqueue(&priority_queues[pri], &processes[process_index]);
            }
            process_index++;
        }

        // Aging: Promote processes waiting 5 quanta (only if aging is enabled)
        if (use_aging) {
            for (int i = PRIORITY_LEVELS - 1; i > 0; i--) { // Start from lowest priority
                for (int j = 0; j < priority_queues[i].count; j++) {
                    int idx = (priority_queues[i].front + j) % 100;
                    RRProcess* p = priority_queues[i].processes[idx];
                    if (current_time - p->waiting_since >= AGING_THRESHOLD && p->priority > 1) {
                        p->priority--;
                        p->waiting_since = current_time;
                        
                        // Move to higher priority queue - need to remove and requeue
                        RRProcess temp = *p;
                        
                        // Remove from current queue
                        for (int k = j; k < priority_queues[i].count - 1; k++) {
                            int curr_idx = (priority_queues[i].front + k) % 100;
                            int next_idx = (priority_queues[i].front + k + 1) % 100;
                            priority_queues[i].processes[curr_idx] = priority_queues[i].processes[next_idx];
                        }
                        priority_queues[i].count--;
                        if (priority_queues[i].count == 0) {
                            priority_queues[i].front = 0;
                            priority_queues[i].rear = -1;
                        } else {
                            priority_queues[i].rear = (priority_queues[i].front + priority_queues[i].count - 1) % 100;
                        }
                        
                        // Add to higher priority queue
                        rr_enqueue(&priority_queues[i-1], &processes[temp.name - 'A']);
                        j--; // Adjust index after removal
                    }
                }
            }
        }

        // Select highest priority non-empty queue
        int selected_queue = -1;
        for (int i = 0; i < PRIORITY_LEVELS; i++) {
            if (priority_queues[i].count > 0) {
                selected_queue = i;
                break;
            }
        }

        if (selected_queue != -1) {
            RRProcess* current = rr_dequeue(&priority_queues[selected_queue]);
            if (current->start_time == -1) {
                current->start_time = current_time;
                current->response_time = current_time - current->arrival_time;
                current->ran = 1;
            }
            
            if (preemptive) {
                // Preemptive: RR within priority queue, 1 quantum at a time
                if (current_time < MAX_QUANTA) {
                    printf("%c", current->name);
                    timeline[current_time] = current->name;
                }
                current->remaining_time--;
                current_time++;
                
                // Check for new arrivals during execution
                while (process_index < process_count && processes[process_index].arrival_time <= current_time) {
                    if (processes[process_index].arrival_time <= MAX_QUANTA) {
                        int pri = processes[process_index].priority - 1;
                        rr_enqueue(&priority_queues[pri], &processes[process_index]);
                    }
                    process_index++;
                }
                
                if (current->remaining_time > 0) {
                    rr_enqueue(&priority_queues[selected_queue], current);
                } else {
                    current->end_time = current_time;
                    completed++;
                }
                
                // Update wait times for all queued processes
                for (int i = 0; i < PRIORITY_LEVELS; i++) {
                    for (int j = 0; j < priority_queues[i].count; j++) {
                        int idx = (priority_queues[i].front + j) % 100;
                        priority_queues[i].processes[idx]->wait_time += 1;
                    }
                }
            } else {
                // Non-preemptive: Run to completion (FCFS)
                for (int t = 0; t < current->remaining_time; t++) {
                    if (current_time + t < MAX_QUANTA) {
                        printf("%c", current->name);
                        timeline[current_time + t] = current->name;
                    }
                }
                
                // Calculate wait time for non-preemptive
                current->wait_time = current->start_time - current->arrival_time;
                
                current->end_time = current_time + current->remaining_time;
                current_time += current->remaining_time;
                current->remaining_time = 0;
                completed++;
                
                // Check for new arrivals during execution
                while (process_index < process_count && processes[process_index].arrival_time <= current_time) {
                    if (processes[process_index].arrival_time <= MAX_QUANTA) {
                        int pri = processes[process_index].priority - 1;
                        rr_enqueue(&priority_queues[pri], &processes[process_index]);
                    }
                    process_index++;
                }
            }
        } else {
            if (current_time < MAX_QUANTA) {
                printf("_");
                timeline[current_time] = '-'; // CPU idle
            }
            current_time++;
        }
    }

    printf("\n");
    
    // Arrays to track per-priority stats
    double priority_response_time[PRIORITY_LEVELS] = {0};
    double priority_wait_time[PRIORITY_LEVELS] = {0};
    double priority_turnaround_time[PRIORITY_LEVELS] = {0};
    int priority_completed[PRIORITY_LEVELS] = {0};
    
    double total_turnaround = 0, total_waiting = 0, total_response = 0;
    int ran_count = 0;
    
    // Calculate statistics for completed processes
    for (int i = 0; i < process_count; i++) {
        if (processes[i].ran && processes[i].start_time >= 0) {
            // Ensure end_time is valid (greater than arrival_time)
            if (processes[i].end_time <= 0) {
                // If end_time isn't set, use current_time
                processes[i].end_time = current_time;
            }
            
            // Make sure turnaround time is positive
            double turnaround = processes[i].end_time - processes[i].arrival_time;
            if (turnaround < 0) {
                turnaround = processes[i].run_time; // Fallback to at least the run time
            }
            
            // Update wait time if not calculated (for non-preemptive)
            if (!preemptive && processes[i].wait_time == 0) {
                processes[i].wait_time = processes[i].start_time - processes[i].arrival_time;
            }
            
            total_turnaround += turnaround;
            total_waiting += processes[i].wait_time;
            total_response += processes[i].response_time;
            ran_count++;
            
            // Track per-priority stats
            int pri = processes[i].priority - 1; // Convert to 0-indexed
            priority_response_time[pri] += processes[i].response_time;
            priority_wait_time[pri] += processes[i].wait_time;
            priority_turnaround_time[pri] += turnaround;
            priority_completed[pri]++;
        }
    }
    
    // Print stats per priority level
    printf("\nStatistics by Priority Level:\n");
    for (int i = 0; i < PRIORITY_LEVELS; i++) {
        if (priority_completed[i] > 0) {
            printf("Priority %d:\n", i + 1);
            printf("  Average Response Time: %.1f\n", 
                   priority_response_time[i] / priority_completed[i]);
            printf("  Average Wait Time: %.1f\n", 
                   priority_wait_time[i] / priority_completed[i]);
            printf("  Average Turnaround Time: %.1f\n", 
                   priority_turnaround_time[i] / priority_completed[i]);
            printf("  Throughput: %.1f processes per 100 quanta\n", 
                   (float)priority_completed[i] / MAX_QUANTA * 100);
        } else {
            printf("Priority %d: No processes completed (possible starvation)\n", i + 1);
        }
    }
    
    RRStats stats = {
        ran_count ? total_turnaround / ran_count : 0,
        ran_count ? total_waiting / ran_count : 0,
        ran_count ? total_response / ran_count : 0,
        ran_count ? (double)ran_count / MAX_QUANTA * 100 : 0  // Fix throughput calculation
    };

    return stats;
}
