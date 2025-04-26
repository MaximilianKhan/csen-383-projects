#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utility.h"
#include "process.h"
#include "stat.h"

// Function declarations from the algorithm files
extern average_stats firstComeFirstServeNP(linked_list *processes);
extern average_stats shortestJobFirstNP(linked_list *processes);
extern average_stats shortestRemainingTimeP(linked_list *processes);
extern average_stats roundRobinP(linked_list *processes, int time_slice);
extern average_stats highestPriorityFirstP(linked_list *processes);
extern average_stats highestPriorityFirstNP(linked_list *processes);
extern average_stats highest_priority_first_p_aging(linked_list *processes);
extern average_stats highest_priority_first_np_aging(linked_list *processes);

// Function to display the processes for debugging
void displayProcesses(linked_list *processList) {
    printf("-------------------------------------------------\n");
    printf("Process ID | Arrival Time | Run Time | Priority |\n");
    printf("-------------------------------------------------\n");
    
    node *processPointer = processList->head;
    while (processPointer != NULL) {
        process *proc = (process *)processPointer->data;
        printf("%10c | %12.1f | %8.1f | %8u |\n", 
               proc->pid, 
               proc->arrivalTime, 
               proc->runTime, 
               proc->priority);
        processPointer = processPointer->next;
    }
    
    printf("--------------------------------------------------\n");
    printf("Total No. of Processes: %d\n", processList->size);
    printf("--------------------------------------------------\n");
}

int main(int argc, char **argv) {
    int roundCounter = 0;
    // Arrays to store statistics for each algorithm over 5 runs
    average_stats fcfs[5], sjf[5], srt[5], rr[5], hpfp[5], hpfnp[5];
    average_stats hpfp_aging[5], hpfnp_aging[5]; // For extra credit
    average_stats final[8]; // For final averages
    
    // Set fixed seed for reproducibility
    srand(3);
    
    // Run 5 times
    while (roundCounter < 5) {
        printf("\n*********************************************************************************\n");
        printf("\t\t\t\tRound %d\n", roundCounter + 1);
        printf("*********************************************************************************\n");
        
        // Generate processes for this round
        linked_list *processList = generateProcesses(52);
        displayProcesses(processList);
        
        // Run FCFS algorithm
        printf("\nRunning First-Come First-Served (FCFS) algorithm...\n");
        fcfs[roundCounter] = firstComeFirstServeNP(processList);
        
        // Run SJF algorithm
        printf("\nRunning Shortest Job First (SJF) algorithm...\n");
        sjf[roundCounter] = shortestJobFirstNP(processList);
        
        // Run SRT algorithm
        printf("\nRunning Shortest Remaining Time (SRT) algorithm...\n");
        srt[roundCounter] = shortestRemainingTimeP(processList);
        
        // Run RR algorithm with time slice of 1
        printf("\nRunning Round Robin (RR) algorithm...\n");
        rr[roundCounter] = roundRobinP(processList, 1);
        
        // Run HPF Preemptive algorithm
        printf("\nRunning Highest Priority First (HPF) Preemptive algorithm...\n");
        hpfp[roundCounter] = highestPriorityFirstP(processList);
        
        // Run HPF Non-Preemptive algorithm
        printf("\nRunning Highest Priority First (HPF) Non-Preemptive algorithm...\n");
        hpfnp[roundCounter] = highestPriorityFirstNP(processList);
        
        // Extra credit: HPF with aging
        printf("\nRunning Highest Priority First (HPF) Preemptive with Aging algorithm...\n");
        hpfp_aging[roundCounter] = highest_priority_first_p_aging(processList);
        
        printf("\nRunning Highest Priority First (HPF) Non-Preemptive with Aging algorithm...\n");
        hpfnp_aging[roundCounter] = highest_priority_first_np_aging(processList);
        
        roundCounter++;
        
        // Free memory for this round's processes
        // Note: In a real implementation, we would need to free the memory properly
    }
    
    // Initialize the final statistics arrays
    for (int i = 0; i < 8; i++) {
        final[i].avg_response_time = 0;
        final[i].avg_wait_time = 0;
        final[i].avg_turnaround = 0;
        final[i].avg_throughput = 0;
    }
    
    // Calculate the average statistics across all 5 runs
    for (int i = 0; i < 5; i++) {
        final[0].avg_response_time += fcfs[i].avg_response_time;
        final[0].avg_wait_time += fcfs[i].avg_wait_time;
        final[0].avg_turnaround += fcfs[i].avg_turnaround;
        final[0].avg_throughput += fcfs[i].avg_throughput;
        
        final[1].avg_response_time += sjf[i].avg_response_time;
        final[1].avg_wait_time += sjf[i].avg_wait_time;
        final[1].avg_turnaround += sjf[i].avg_turnaround;
        final[1].avg_throughput += sjf[i].avg_throughput;
        
        final[2].avg_response_time += srt[i].avg_response_time;
        final[2].avg_wait_time += srt[i].avg_wait_time;
        final[2].avg_turnaround += srt[i].avg_turnaround;
        final[2].avg_throughput += srt[i].avg_throughput;
        
        final[3].avg_response_time += rr[i].avg_response_time;
        final[3].avg_wait_time += rr[i].avg_wait_time;
        final[3].avg_turnaround += rr[i].avg_turnaround;
        final[3].avg_throughput += rr[i].avg_throughput;
        
        final[4].avg_response_time += hpfp[i].avg_response_time;
        final[4].avg_wait_time += hpfp[i].avg_wait_time;
        final[4].avg_turnaround += hpfp[i].avg_turnaround;
        final[4].avg_throughput += hpfp[i].avg_throughput;
        
        final[5].avg_response_time += hpfnp[i].avg_response_time;
        final[5].avg_wait_time += hpfnp[i].avg_wait_time;
        final[5].avg_turnaround += hpfnp[i].avg_turnaround;
        final[5].avg_throughput += hpfnp[i].avg_throughput;
        
        final[6].avg_response_time += hpfp_aging[i].avg_response_time;
        final[6].avg_wait_time += hpfp_aging[i].avg_wait_time;
        final[6].avg_turnaround += hpfp_aging[i].avg_turnaround;
        final[6].avg_throughput += hpfp_aging[i].avg_throughput;
        
        final[7].avg_response_time += hpfnp_aging[i].avg_response_time;
        final[7].avg_wait_time += hpfnp_aging[i].avg_wait_time;
        final[7].avg_turnaround += hpfnp_aging[i].avg_turnaround;
        final[7].avg_throughput += hpfnp_aging[i].avg_throughput;
    }
    
    // Calculate the final averages
    for (int i = 0; i < 8; i++) {
        final[i].avg_response_time /= 5;
        final[i].avg_wait_time /= 5;
        final[i].avg_turnaround /= 5;
        final[i].avg_throughput /= 5;
    }
    
    // Print the final results
    printf("\n\n");
    printf("_____________________________________________________________________________________________________________________________\n\n");
    printf("The average of the 5 runs of every algorithm is as follows:\n");
    printf("\n");
    
    // Display average statistics for each algorithm
    printf("ALGORITHM: First-come First-served (FCFS) [non-preemptive]:\n");
    printf("Average Response Time: %.1f\n", final[0].avg_response_time);
    printf("Average Wait Time: %.1f\n", final[0].avg_wait_time);
    printf("Average Turn Around Time: %.1f\n", final[0].avg_turnaround);
    printf("Average Throughput: %.1f\n", final[0].avg_throughput);
    printf("\n");
    
    printf("ALGORITHM: Shortest Job First (SJF) [non-preemptive]:\n");
    printf("Average Response Time: %.1f\n", final[1].avg_response_time);
    printf("Average Wait Time: %.1f\n", final[1].avg_wait_time);
    printf("Average Turn Around Time: %.1f\n", final[1].avg_turnaround);
    printf("Average Throughput: %.1f\n", final[1].avg_throughput);
    printf("\n");
    
    printf("ALGORITHM: Shortest Remaining Time (SRT) [preemptive]:\n");
    printf("Average Response Time: %.1f\n", final[2].avg_response_time);
    printf("Average Wait Time: %.1f\n", final[2].avg_wait_time);
    printf("Average Turn Around Time: %.1f\n", final[2].avg_turnaround);
    printf("Average Throughput: %.1f\n", final[2].avg_throughput);
    printf("\n");
    
    printf("ALGORITHM: Round Robin (RR) [preemptive]:\n");
    printf("Average Response Time: %.1f\n", final[3].avg_response_time);
    printf("Average Wait Time: %.1f\n", final[3].avg_wait_time);
    printf("Average Turn Around Time: %.1f\n", final[3].avg_turnaround);
    printf("Average Throughput: %.1f\n", final[3].avg_throughput);
    printf("\n");
    
    printf("ALGORITHM: Highest Priority First (HPF) [preemptive]:\n");
    printf("Average Response Time: %.1f\n", final[4].avg_response_time);
    printf("Average Wait Time: %.1f\n", final[4].avg_wait_time);
    printf("Average Turn Around Time: %.1f\n", final[4].avg_turnaround);
    printf("Average Throughput: %.1f\n", final[4].avg_throughput);
    printf("\n");
    
    printf("ALGORITHM: Highest Priority First (HPF) [non-preemptive]:\n");
    printf("Average Response Time: %.1f\n", final[5].avg_response_time);
    printf("Average Wait Time: %.1f\n", final[5].avg_wait_time);
    printf("Average Turn Around Time: %.1f\n", final[5].avg_turnaround);
    printf("Average Throughput: %.1f\n", final[5].avg_throughput);
    printf("\n");
    
    printf("EXTRA CREDIT - ALGORITHM: Highest Priority First (HPF) with Aging [preemptive]:\n");
    printf("Average Response Time: %.1f\n", final[6].avg_response_time);
    printf("Average Wait Time: %.1f\n", final[6].avg_wait_time);
    printf("Average Turn Around Time: %.1f\n", final[6].avg_turnaround);
    printf("Average Throughput: %.1f\n", final[6].avg_throughput);
    printf("\n");
    
    printf("EXTRA CREDIT - ALGORITHM: Highest Priority First (HPF) with Aging [non-preemptive]:\n");
    printf("Average Response Time: %.1f\n", final[7].avg_response_time);
    printf("Average Wait Time: %.1f\n", final[7].avg_wait_time);
    printf("Average Turn Around Time: %.1f\n", final[7].avg_turnaround);
    printf("Average Throughput: %.1f\n", final[7].avg_throughput);
    printf("\n");
    
    return 0;
}
