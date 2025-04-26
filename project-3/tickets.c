#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

// Constants
#define NUM_SEATS 100
#define NUM_ROWS 10
#define SEATS_PER_ROW 10
#define NUM_H_SELLERS 1
#define NUM_M_SELLERS 3
#define NUM_L_SELLERS 6
#define TOTAL_SELLERS (NUM_H_SELLERS + NUM_M_SELLERS + NUM_L_SELLERS)
#define MAX_MINUTES 60

// Synchronization primitives
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t seat_mutex = PTHREAD_MUTEX_INITIALIZER;

// Shared data
int seats[NUM_ROWS][SEATS_PER_ROW]; // 0: unsold, 1: sold
char seat_owners[NUM_ROWS][SEATS_PER_ROW][5]; // Customer IDs (e.g., H001)
int total_seats_sold = 0;
int current_time = 0;
int sold_out = 0;
int N; // Customers per seller

// Statistics
int h_served = 0, m_served = 0, l_served = 0;
int h_turned_away = 0, m_turned_away = 0, l_turned_away = 0;
double h_response_sum = 0, m_response_sum = 0, l_response_sum = 0;
double h_turnaround_sum = 0, m_turnaround_sum = 0, l_turnaround_sum = 0;

// Customer node for linked list
typedef struct Customer {
    int arrival_time;
    char id[5]; // e.g., H001, M101
    int service_time;
    int start_time; // Time when service starts
    struct Customer* next;
} Customer;

// Seller structure
typedef struct {
    char type; // 'H', 'M', 'L'
    int index; // 0 for H, 1-3 for M, 1-6 for L
    Customer* queue; // Head of customer linked list
    int is_busy; // 1 if serving, 0 if idle
    Customer* current_customer; // Customer being served
    int time_remaining; // Remaining service time
} Seller;

Seller sellers[TOTAL_SELLERS];

// Create a customer
Customer* create_customer(char type, int index, int customer_num, int arrival_time) {
    Customer* c = malloc(sizeof(Customer));
    c->arrival_time = arrival_time;
    c->service_time = (type == 'H') ? (rand() % 2 + 1) : (type == 'M') ? (rand() % 3 + 2) : (rand() % 4 + 4);
    if (type == 'H') snprintf(c->id, 5, "H%03d", customer_num);
    else if (type == 'M') snprintf(c->id, 5, "M%d%02d", index, customer_num);
    else snprintf(c->id, 5, "L%d%02d", index, customer_num);
    c->start_time = -1;
    c->next = NULL;
    return c;
}

// Insert customer into queue sorted by arrival time
void insert_customer(Customer** head, Customer* new_customer) {
    if (!*head || new_customer->arrival_time < (*head)->arrival_time) {
        new_customer->next = *head;
        *head = new_customer;
        return;
    }
    Customer* current = *head;
    while (current->next && current->next->arrival_time <= new_customer->arrival_time) {
        current = current->next;
    }
    new_customer->next = current->next;
    current->next = new_customer;
}

// Assign seat
int assign_seat(char type, int index, char* customer_id, int* row, int* col) {
    pthread_mutex_lock(&seat_mutex);
    if (total_seats_sold >= NUM_SEATS) {
        pthread_mutex_unlock(&seat_mutex);
        return 0;
    }
    int assigned = 0;
    if (type == 'H') {
        for (int r = 0; r < NUM_ROWS && !assigned; r++) {
            for (int c = 0; c < SEATS_PER_ROW; c++) {
                if (seats[r][c] == 0) {
                    seats[r][c] = 1;
                    strcpy(seat_owners[r][c], customer_id);
                    *row = r;
                    *col = c;
                    total_seats_sold++;
                    assigned = 1;
                    break;
                }
            }
        }
    } else if (type == 'L') {
        for (int r = NUM_ROWS - 1; r >= 0 && !assigned; r--) {
            for (int c = 0; c < SEATS_PER_ROW; c++) {
                if (seats[r][c] == 0) {
                    seats[r][c] = 1;
                    strcpy(seat_owners[r][c], customer_id);
                    *row = r;
                    *col = c;
                    total_seats_sold++;
                    assigned = 1;
                    break;
                }
            }
        }
    } else {
        int rows[] = {4, 5, 3, 6, 2, 7, 1, 8, 0, 9};
        for (int i = 0; i < NUM_ROWS && !assigned; i++) {
            int r = rows[i];
            for (int c = 0; c < SEATS_PER_ROW; c++) {
                if (seats[r][c] == 0) {
                    seats[r][c] = 1;
                    strcpy(seat_owners[r][c], customer_id);
                    *row = r;
                    *col = c;
                    total_seats_sold++;
                    assigned = 1;
                    break;
                }
            }
        }
    }
    if (total_seats_sold >= NUM_SEATS) sold_out = 1;
    pthread_mutex_unlock(&seat_mutex);
    return assigned;
}

// Print seating chart
void print_seating_chart() {
    printf("Seating Chart:\n");
    for (int r = 0; r < NUM_ROWS; r++) {
        for (int c = 0; c < SEATS_PER_ROW; c++) {
            printf("%s ", seats[r][c] ? seat_owners[r][c] : "----");
        }
        printf("\n");
    }
    printf("\n");
}

// Seller thread function
void* sell(void* arg) {
    Seller* seller = (Seller*)arg;
    char seller_name[3];
    if (seller->type == 'H') snprintf(seller_name, 3, "H");
    else snprintf(seller_name, 3, "%c%d", seller->type, seller->index);

    while (current_time < MAX_MINUTES && !sold_out) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock(&mutex);

        // Check for new arrivals
        Customer* current = seller->queue;
        while (current && current->arrival_time == current_time) {
            printf("%d:%02d Customer %s arrives at %s's queue\n", current_time / 60, current_time % 60, current->id, seller_name);
            current = current->next;
        }

        // If busy, continue serving current customer
        if (seller->is_busy) {
            seller->time_remaining--;
            if (seller->time_remaining == 0) {
                printf("%d:%02d Customer %s completes purchase and leaves\n", current_time / 60, current_time % 60, seller->current_customer->id);
                double response_time = seller->current_customer->start_time - seller->current_customer->arrival_time;
                double turnaround_time = response_time + seller->current_customer->service_time;
                if (seller->type == 'H') {
                    h_served++;
                    h_response_sum += response_time;
                    h_turnaround_sum += turnaround_time;
                } else if (seller->type == 'M') {
                    m_served++;
                    m_response_sum += response_time;
                    m_turnaround_sum += turnaround_time;
                } else {
                    l_served++;
                    l_response_sum += response_time;
                    l_turnaround_sum += turnaround_time;
                }
                free(seller->current_customer);
                seller->current_customer = NULL;
                seller->is_busy = 0;
            }
        }

        // If not busy, serve next customer
        if (!seller->is_busy && seller->queue && seller->queue->arrival_time <= current_time) {
            seller->current_customer = seller->queue;
            seller->queue = seller->queue->next;
            seller->current_customer->next = NULL;
            seller->is_busy = 1;
            seller->time_remaining = seller->current_customer->service_time;
            seller->current_customer->start_time = current_time;
            printf("%d:%02d %s starts serving customer %s\n", current_time / 60, current_time % 60, seller_name, seller->current_customer->id);
            int row, col;
            if (assign_seat(seller->type, seller->index, seller->current_customer->id, &row, &col)) {
                printf("%d:%02d Customer %s assigned seat (%d,%d)\n", current_time / 60, current_time % 60, seller->current_customer->id, row + 1, col + 1);
                print_seating_chart();
            } else {
                printf("%d:%02d Customer %s turned away (sold out)\n", current_time / 60, current_time % 60, seller->current_customer->id);
                if (seller->type == 'H') h_turned_away++;
                else if (seller->type == 'M') m_turned_away++;
                else l_turned_away++;
                free(seller->current_customer);
                seller->current_customer = NULL;
                seller->is_busy = 0;
            }
        }
    }

    // Handle remaining customers
    Customer* current = seller->queue;
    while (current) {
        printf("%d:%02d Customer %s leaves queue (window closed)\n", current_time / 60, current_time % 60, current->id);
        if (seller->type == 'H') h_turned_away++;
        else if (seller->type == 'M') m_turned_away++;
        else l_turned_away++;
        Customer* temp = current;
        current = current->next;
        free(temp);
    }
    seller->queue = NULL;
    if (seller->is_busy) {
        printf("%d:%02d Customer %s completes purchase and leaves\n", current_time / 60, current_time % 60, seller->current_customer->id);
        double response_time = seller->current_customer->start_time - seller->current_customer->arrival_time;
        double turnaround_time = response_time + seller->current_customer->service_time;
        if (seller->type == 'H') {
            h_served++;
            h_response_sum += response_time;
            h_turnaround_sum += turnaround_time;
        } else if (seller->type == 'M') {
            m_served++;
            m_response_sum += response_time;
            m_turnaround_sum += turnaround_time;
        } else {
            l_served++;
            l_response_sum += response_time;
            l_turnaround_sum += turnaround_time;
        }
        free(seller->current_customer);
    }
    return NULL;
}

// Wake up all seller threads
void wakeup_all_seller_threads() {
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <N>\n", argv[0]);
        return 1;
    }
    N = atoi(argv[1]);
    srand(time(NULL));

    // Initialize seats
    for (int r = 0; r < NUM_ROWS; r++)
        for (int c = 0; c < SEATS_PER_ROW; c++)
            strcpy(seat_owners[r][c], "----");

    // Initialize sellers and customer queues
    int seller_idx = 0;
    // H seller
    sellers[seller_idx].type = 'H';
    sellers[seller_idx].index = 0;
    sellers[seller_idx].queue = NULL;
    sellers[seller_idx].is_busy = 0;
    for (int i = 1; i <= N; i++) {
        Customer* c = create_customer('H', 0, i, rand() % MAX_MINUTES);
        insert_customer(&sellers[seller_idx].queue, c);
    }
    seller_idx++;

    // M sellers
    for (int i = 1; i <= NUM_M_SELLERS; i++) {
        sellers[seller_idx].type = 'M';
        sellers[seller_idx].index = i;
        sellers[seller_idx].queue = NULL;
        sellers[seller_idx].is_busy = 0;
        for (int j = 1; j <= N; j++) {
            Customer* c = create_customer('M', i, j, rand() % MAX_MINUTES);
            insert_customer(&sellers[seller_idx].queue, c);
        }
        seller_idx++;
    }

    // L sellers
    for (int i = 1; i <= NUM_L_SELLERS; i++) {
        sellers[seller_idx].type = 'L';
        sellers[seller_idx].index = i;
        sellers[seller_idx].queue = NULL;
        sellers[seller_idx].is_busy = 0;
        for (int j = 1; j <= N; j++) {
            Customer* c = create_customer('L', i, j, rand() % MAX_MINUTES);
            insert_customer(&sellers[seller_idx].queue, c);
        }
        seller_idx++;
    }

    // Create seller threads
    pthread_t tids[TOTAL_SELLERS];
    seller_idx = 0;
    pthread_create(&tids[0], NULL, sell, &sellers[seller_idx++]);
    for (int i = 1; i <= NUM_M_SELLERS; i++)
        pthread_create(&tids[i], NULL, sell, &sellers[seller_idx++]);
    for (int i = NUM_M_SELLERS + 1; i < TOTAL_SELLERS; i++)
        pthread_create(&tids[i], NULL, sell, &sellers[seller_idx++]);

    // Simulate time
    for (current_time = 0; current_time < MAX_MINUTES && !sold_out; current_time++) {
        wakeup_all_seller_threads();
        sleep(1); // 1 minute
    }

    // Wake up threads to handle remaining customers
    wakeup_all_seller_threads();

    // Wait for all seller threads to exit
    for (int i = 0; i < TOTAL_SELLERS; i++)
        pthread_join(tids[i], NULL);

    // Print simulation results
    printf("\nFinal Statistics:\n");
    printf("H Customers Served: %d, Turned Away: %d\n", h_served, h_turned_away);
    printf("M Customers Served: %d, Turned Away: %d\n", m_served, m_turned_away);
    printf("L Customers Served: %d, Turned Away: %d\n", l_served, l_turned_away);
    printf("Average Response Time (H): %.2f minutes\n", h_served ? h_response_sum / h_served : 0);
    printf("Average Response Time (M): %.2f minutes\n", m_served ? m_response_sum / m_served : 0);
    printf("Average Response Time (L): %.2f minutes\n", l_served ? l_response_sum / l_served : 0);
    printf("Average Turnaround Time (H): %.2f minutes\n", h_served ? h_turnaround_sum / h_served : 0);
    printf("Average Turnaround Time (M): %.2f minutes\n", m_served ? m_turnaround_sum / m_served : 0);
    printf("Average Turnaround Time (L): %.2f minutes\n", l_served ? l_turnaround_sum / l_served : 0);
    printf("Throughput (H): %.2f customers/minute\n", h_served / (float)MAX_MINUTES);
    printf("Throughput (M): %.2f customers/minute\n", m_served / (float)MAX_MINUTES);
    printf("Throughput (L): %.2f customers/minute\n", l_served / (float)MAX_MINUTES);

    // Cleanup
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&seat_mutex);
    pthread_cond_destroy(&cond);

    exit(0);
}