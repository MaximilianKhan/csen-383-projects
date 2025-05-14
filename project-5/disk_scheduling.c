#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CYLINDERS 5000
#define REQUESTS 10

int abs_diff(int a, int b) {
    return abs(a - b);
}

void copy_requests(int src[], int dst[], int size) {
    for (int i = 0; i < size; i++) {
        dst[i] = src[i];
    }
}

// FCFS: Process requests in the order they arrive
void fcfs(int head, int requests[], int order[], int *total_distance) {
    int current = head;
    *total_distance = 0;
    for (int i = 0; i < REQUESTS; i++) {
        order[i] = requests[i];
        *total_distance += abs_diff(current, requests[i]);
        current = requests[i];
    }
}

// SSTF: Serve the closest request next
void sstf(int head, int requests[], int order[], int *total_distance) {
    int temp[REQUESTS];
    copy_requests(requests, temp, REQUESTS);
    int visited[REQUESTS] = {0};
    int current = head;
    *total_distance = 0;

    for (int i = 0; i < REQUESTS; i++) {
        int min_dist = CYLINDERS + 1;
        int min_idx = -1;

        // Find closest unvisited request
        for (int j = 0; j < REQUESTS; j++) {
            if (!visited[j]) {
                int dist = abs_diff(current, temp[j]);
                if (dist < min_dist) {
                    min_dist = dist;
                    min_idx = j;
                }
            }
        }

        visited[min_idx] = 1;
        order[i] = temp[min_idx];
        *total_distance += min_dist;
        current = temp[min_idx];
    }
}

// SCAN: Move from 0 to max, then reverse
void scan(int head, int requests[], int order[], int *total_distance, int prev) {
    int temp[REQUESTS + 2]; // Include head and boundaries if needed
    int direction = (head > prev) ? 1 : -1; // Determine initial direction
    int idx = 0;

    // Sort requests
    copy_requests(requests, temp, REQUESTS);
    for (int i = 0; i < REQUESTS - 1; i++) {
        for (int j = 0; j < REQUESTS - i - 1; j++) {
            if (temp[j] > temp[j + 1]) {
                int t = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = t;
            }
        }
    }

    *total_distance = 0;
    int current = head;
    int order_idx = 0;

    if (direction == 1) {
        // Move right (towards 4999)
        for (int i = 0; i < REQUESTS; i++) {
            if (temp[i] >= head) {
                order[order_idx++] = temp[i];
                *total_distance += abs_diff(current, temp[i]);
                current = temp[i];
            }
        }
        // Move to 4999
        *total_distance += abs_diff(current, 4999);
        current = 4999;
        // Move left (towards 0)
        for (int i = REQUESTS - 1; i >= 0; i--) {
            if (temp[i] < head) {
                order[order_idx++] = temp[i];
                *total_distance += abs_diff(current, temp[i]);
                current = temp[i];
            }
        }
    } else {
        // Move left (towards 0)
        for (int i = REQUESTS - 1; i >= 0; i--) {
            if (temp[i] <= head) {
                order[order_idx++] = temp[i];
                *total_distance += abs_diff(current, temp[i]);
                current = temp[i];
            }
        }
        // Move to 0
        *total_distance += abs_diff(current, 0);
        current = 0;
        // Move right (towards 4999)
        for (int i = 0; i < REQUESTS; i++) {
            if (temp[i] > head) {
                order[order_idx++] = temp[i];
                *total_distance += abs_diff(current, temp[i]);
                current = temp[i];
            }
        }
    }
}

// LOOK: Like SCAN but reverses when no more requests in direction
void look(int head, int requests[], int order[], int *total_distance, int prev) {
    int temp[REQUESTS];
    copy_requests(requests, temp, REQUESTS);
    int direction = (head > prev) ? 1 : -1;
    int idx = 0;

    // Sort requests
    for (int i = 0; i < REQUESTS - 1; i++) {
        for (int j = 0; j < REQUESTS - i - 1; j++) {
            if (temp[j] > temp[j + 1]) {
                int t = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = t;
            }
        }
    }

    *total_distance = 0;
    int current = head;
    int order_idx = 0;

    if (direction == 1) {
        // Move right
        for (int i = 0; i < REQUESTS; i++) {
            if (temp[i] >= head) {
                order[order_idx++] = temp[i];
                *total_distance += abs_diff(current, temp[i]);
                current = temp[i];
            }
        }
        // Move left to remaining requests
        for (int i = REQUESTS - 1; i >= 0; i--) {
            if (temp[i] < head) {
                order[order_idx++] = temp[i];
                *total_distance += abs_diff(current, temp[i]);
                current = temp[i];
            }
        }
    } else {
        // Move left
        for (int i = REQUESTS - 1; i >= 0; i--) {
            if (temp[i] <= head) {
                order[order_idx++] = temp[i];
                *total_distance += abs_diff(current, temp[i]);
                current = temp[i];
            }
        }
        // Move right
        for (int i = 0; i < REQUESTS; i++) {
            if (temp[i] > head) {
                order[order_idx++] = temp[i];
                *total_distance += abs_diff(current, temp[i]);
                current = temp[i];
            }
        }
    }
}

// C-SCAN: Move in one direction (right), wrap around to 0
void cscan(int head, int requests[], int order[], int *total_distance) {
    int temp[REQUESTS];
    copy_requests(requests, temp, REQUESTS);
    int idx = 0;

    // Sort requests
    for (int i = 0; i < REQUESTS - 1; i++) {
        for (int j = 0; j < REQUESTS - i - 1; j++) {
            if (temp[j] > temp[j + 1]) {
                int t = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = t;
            }
        }
    }

    *total_distance = 0;
    int current = head;
    int order_idx = 0;

    // Move right to 4999
    for (int i = 0; i < REQUESTS; i++) {
        if (temp[i] >= head) {
            order[order_idx++] = temp[i];
            *total_distance += abs_diff(current, temp[i]);
            current = temp[i];
        }
    }
    // Jump to 0 (wrap around)
    *total_distance += abs_diff(current, 4999) + abs_diff(4999, 0);
    current = 0;
    // Serve remaining requests
    for (int i = 0; i < REQUESTS; i++) {
        if (temp[i] < head) {
            order[order_idx++] = temp[i];
            *total_distance += abs_diff(current, temp[i]);
            current = temp[i];
        }
    }
}

// C-LOOK: Move right to max request, then jump to min request
void clook(int head, int requests[], int order[], int *total_distance) {
    int temp[REQUESTS];
    copy_requests(requests, temp, REQUESTS);

    // Sort requests
    for (int i = 0; i < REQUESTS - 1; i++) {
        for (int j = 0; j < REQUESTS - i - 1; j++) {
            if (temp[j] > temp[j + 1]) {
                int t = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = t;
            }
        }
    }

    *total_distance = 0;
    int current = head;
    int order_idx = 0;

    // Move right to max request
    for (int i = 0; i < REQUESTS; i++) {
        if (temp[i] >= head) {
            order[order_idx++] = temp[i];
            *total_distance += abs_diff(current, temp[i]);
            current = temp[i];
        }
    }

    // Find the minimum request < head
    int min_request = CYLINDERS;
    for (int i = 0; i < REQUESTS; i++) {
        if (temp[i] < head && temp[i] < min_request) {
            min_request = temp[i];
        }
    }

    // Jump to min request and serve remaining requests
    if (min_request != CYLINDERS) {
        *total_distance += abs_diff(current, min_request);
        current = min_request;
        order[order_idx++] = min_request;
        // Serve remaining requests < head in ascending order
        for (int i = 0; i < REQUESTS; i++) {
            if (temp[i] < head && temp[i] != min_request) {
                order[order_idx++] = temp[i];
                *total_distance += abs_diff(current, temp[i]);
                current = temp[i];
            }
        }
    }
}

// Print order and distance
void print_results(FILE *fp, const char *algo, int order[], int total_distance) {
    fprintf(fp, "\n%s:\n", algo);
    fprintf(fp, "Order: ");
    for (int i = 0; i < REQUESTS; i++) {
        fprintf(fp, "%d ", order[i]);
    }
    fprintf(fp, "\nTotal Distance: %d cylinders\n", total_distance);
}

int main() {
    int head = 2255;
    int prev = 1723;
    int requests[REQUESTS] = {2055, 1175, 2304, 2700, 513, 1680, 256, 1401, 4922, 3692};
    int order[REQUESTS];
    int total_distance;

    FILE *fp = fopen("disk_scheduling_results.txt", "w");
    if (!fp) {
        perror("Error opening file");
        return -1;
    }

    fprintf(fp, "Disk Scheduling Algorithms Simulation\n");
    fprintf(fp, "Initial Head Position: %d\n", head);
    fprintf(fp, "Previous Request: %d\n", prev);

    fcfs(head, requests, order, &total_distance);
    print_results(fp, "FCFS", order, total_distance);

    sstf(head, requests, order, &total_distance);
    print_results(fp, "SSTF", order, total_distance);

    scan(head, requests, order, &total_distance, prev);
    print_results(fp, "SCAN", order, total_distance);

    look(head, requests, order, &total_distance, prev);
    print_results(fp, "LOOK", order, total_distance);

    cscan(head, requests, order, &total_distance);
    print_results(fp, "C-SCAN", order, total_distance);

    clook(head, requests, order, &total_distance);
    print_results(fp, "C-LOOK", order, total_distance);

    fclose(fp);
    printf("Results written to disk_scheduling_results.txt\n");
    return 0;
}