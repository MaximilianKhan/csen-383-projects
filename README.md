# csen-383-projects


## Project 3

This project implements a multi-threaded ticket sellers simulation in C using the Pthreads library. The program simulates 10 ticket sellers (1 high-priced [H], 3 medium-priced [M1, M2, M3], and 6 low-priced [L1-L6]) selling 100 concert seats in a 10x10 grid over a one-hour period. Each seller handles a queue of N customers (specified via command-line), who arrive at random times (0-59 minutes). Sellers assign seats based on their type: H starts from row 1 (front), L from row 10 (back), and M from row 5 outward. The simulation ensures no seat is double-booked using mutex locks and tracks events with timestamps.

Key features:
- **Customer Service**: High-priced customers require 1-2 minutes, medium 2-4 minutes, low 4-7 minutes.
- **Events**: Logs customer arrivals, service starts, seat assignments, completions, and turn-aways (if sold out).
- **Seating Chart**: Prints a 10x10 matrix after each sale, showing customer IDs (e.g., H001, M101) or unsold seats (----).
- **Statistics**: Computes average response time (wait time), turnaround time (wait + service), and throughput per seller type.
- **Synchronization**: Uses mutexes for seat assignments and condition variables to wake sellers each minute.

The simulation runs for 60 minutes or until all seats are sold. At the end, remaining customers are turned away, and final statistics are printed.

### Build and Run

The provided `Makefile` in the project directory automates compilation and execution:
1. **Prerequisites**: Ensure `gcc` and `make` are installed (`xcode-select --install` on macOS).
2. **Build and Run**:
   ```bash
   make run