#include "Page.h"
#include <time.h>

int main(int arg1, char* arg2[]) 
{
    int timeStamp = 0;  // Time stamp simulator
    int *pageCountOPT = malloc(sizeof(int)*4);
    char message[100];
    pageCountOPT[0] = 5;
    pageCountOPT[1] = 11;
    pageCountOPT[2] = 17;
    pageCountOPT[3] = 31;

    page *pagePointer;
    page *(*algoFunct)(LISTOFPAGES*);

    if (arg1 != 2)
    {
        printf("Page replacement algorithm is missing in command line. Allowed algorithms are: FIFO, LRU, LFU, MFU or Random.\n");
        return -1;
    }

    // Map algorithm name to function
    if(strcmp(arg2[1], "FIFO") == 0) algoFunct = FirstInFirstOut;
    else if(strcmp(arg2[1], "LRU") == 0) algoFunct = LeastRecentlyUsed;
    else if(strcmp(arg2[1], "LFU") == 0) algoFunct = LeastFrequentlyUsed;
    else if(strcmp(arg2[1], "MFU") == 0) algoFunct = MostFrequentlyUsed;
    else if(strcmp(arg2[1], "Random") == 0) algoFunct = RandomPageReplacement;
    else {
        printf("--------------------------------------------------------------------\n");
        printf("Available algorithms for simulation : FIFO, LRU, LFU, MFU or Random.\n");
        printf("--------------------------------------------------------------------\n");
        return -1;
    }

    char filename[50];
    sprintf(filename, "logs_%s.txt", arg2[1]);
    FILE *log = fopen(filename, "w");
    if (!log) {
        perror("Error opening log file");
        return -1;
    }

    srand(time(NULL));

    for(int i = 0; i < SIMULATION_COUNT; i++) 
    {
        fprintf(log, "******************** Simulator Run %d *****************\n", i+1);
        LISTOFPAGES pl;
        InitPageList(&pl);
        process Q[TOTAL_PROCESS];

        for(int i = 0; i < TOTAL_PROCESS; i++) 
        {
            Q[i].pid = i;
            Q[i].pageCounter = pageCountOPT[rand() % 4];
            Q[i].arrivalTime = rand() % 60;
            Q[i].serviceDuration =  1 + rand() % PROCESS_DURATION;
            Q[i].originalDuration =  Q[i].serviceDuration;
            Q[i].pageReference = 0;
        }

        qsort(Q, TOTAL_PROCESS, sizeof(process), CompArrTime);

        int index = 0, pagesSwappedIn = 0, totalPagesReferenced = 0, pagesAlreadyInMemory = 0, processesStarted = 0;

        for(timeStamp = 0; timeStamp < TOTAL_DURATION; timeStamp++) 
        {
            while(index < TOTAL_PROCESS && Q[index].arrivalTime <= timeStamp) 
            {
                if(FindFreePages(&pl, 4)) 
                {
                    page* p = PageFree(&pl);
                    p->pid = Q[index].pid;
                    p->pageNumber = Q[index].pageReference;
                    p->timeBought = 1.0 * timeStamp;
                    p->counter = 1;
                    p->loadTime = timeStamp;
                    fprintf(log, "timestamp: %.1f sec, process: %3d, pages: %2d, duration: %d, status: entering\n",
                            p->timeBought, p->pid, Q[index].pageCounter, Q[index].originalDuration);
                    LogMemoryMap(log, &pl);
                    index++;
                    pagesSwappedIn++;
                    totalPagesReferenced++;
                    processesStarted++;
                } else {
                    break;
                }
            }

            for(int i = 0; i < 10; i++) 
            {
                for(int j = 0; j < index; j++) {
                    if(Q[j].serviceDuration > 0) {
                        Q[j].pageReference = GenNextPageNumber(Q[j].pageReference, Q[j].pageCounter);

                        if(PagesInMemory(&pl, Q[j].pid, Q[j].pageReference)) {
                            pagePointer = PageIDFree(&pl, Q[j].pid, Q[j].pageReference);
                            if (pagePointer == NULL) {
                                fprintf(log, "ERROR: NULL page for pid %d, page %d\n", Q[j].pid, Q[j].pageReference);
                                return -1;
                            }
                            pagePointer->counter++;
                            pagePointer->loadTime = timeStamp + (0.1 * i);
                            DisplayStatusToFile(log, pagePointer, pagePointer->loadTime, "page already in memory");
                            pagesAlreadyInMemory++;
                            totalPagesReferenced++;
                            continue;
                        }

                        page* pge = PageFree(&pl);
                        if(!pge) {
                            pge = algoFunct(&pl);
                            sprintf(message, "Page %d from Process %d was evicted using %s", pge->pageNumber, pge->pid, arg2[1]);
                        } else {
                            sprintf(message, "loaded into free page");
                        }

                        pge->pid = Q[j].pid;
                        pge->pageNumber = Q[j].pageReference;
                        pge->timeBought = timeStamp + (0.1 * i);
                        pge->loadTime = pge->timeBought;
                        pge->counter = 0;

                        DisplayStatusToFile(log, pge, pge->timeBought, message);
                        pagesSwappedIn++;
                        totalPagesReferenced++;
                    }
                }
            }

            for(int j = 0; j < index; j++) if(Q[j].serviceDuration > 0) 
            {
                Q[j].serviceDuration--;
                if(Q[j].serviceDuration == 0) 
                {
                    FreeMemory(&pl, Q[j].pid);
                    fprintf(log, "timestamp: %2d.0 sec, process: %3d, pages: %2d, duration: %d, status: exiting\n",
                            timeStamp + 1, Q[j].pid, Q[j].pageCounter, Q[j].originalDuration);
                LogMemoryMap(log, &pl);

                }
            }
        }

        fprintf(log, "Run %d Summary:\n", i+1);
        fprintf(log, "Processes Started: %d\n", processesStarted);
        fprintf(log, "Pages Already In Memory: %d\n", pagesAlreadyInMemory);
        fprintf(log, "Pages Swapped In: %d\n", pagesSwappedIn);
        fprintf(log, "Total Pages Referenced: %d\n", totalPagesReferenced);
        fprintf(log, "Hit Ratio: %.2f%%\n", (pagesAlreadyInMemory * 100.0 / totalPagesReferenced));
        fprintf(log, "Miss Ratio: %.2f%%\n\n", (pagesSwappedIn * 100.0 / totalPagesReferenced));
    }

    fclose(log);
    return 0;
}
