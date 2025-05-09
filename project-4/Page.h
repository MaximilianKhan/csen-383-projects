#ifndef PAGING_H
#define PAGING_H

#define DEBUG 0

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define TOTAL_PROCESS 150
#define TOTAL_DURATION 60
#define PROCESS_DURATION 5
#define TOTAL_PAGE 100

#define SIMULATION_COUNT 5

extern int simulationClock;
extern int *pagingOPT;

typedef struct 
{
    int pid, pageCounter, arrivalTime, serviceDuration, originalDuration, pageReference;
} process;

typedef struct page 
{
    int pid;
    int pageNumber;
    struct page* next;
    float timeBought;
    float loadTime;
    int counter;
} page;

typedef struct 
{
    page* head;
} LISTOFPAGES;

//Pages which are free
int FindFreePages(LISTOFPAGES*,int);

//Existing pages in memory
int PagesInMemory(LISTOFPAGES*,int,int);

page* PageFree(LISTOFPAGES*);

//Freeing memory
int FreeMemory(LISTOFPAGES*,int);

//Initializing all pages
void InitPageList(LISTOFPAGES*);

//Displaying all pages
void DisplayAllPage(LISTOFPAGES*);

//Logs the current state of memory pages to the output file
void LogMemoryMap(FILE *log, LISTOFPAGES *pl);

//Generate next page number
int GenNextPageNumber(int,int);

//Comparision between arrival time
int CompArrTime(const void* ,const void*);

// Page status
void DisplayStatus(page *p, float timestamp, char *status);

// File-based logging
void DisplayStatusToFile(FILE *log, page *p, float timestamp, char *status);

//Pages id who are free
page* PageIDFree(LISTOFPAGES*,int,int);

page *FirstInFirstOut(LISTOFPAGES*);
page *LeastRecentlyUsed(LISTOFPAGES*);
page *LeastFrequentlyUsed(LISTOFPAGES*);
page *MostFrequentlyUsed(LISTOFPAGES*);
page *RandomPageReplacement(LISTOFPAGES*);

#endif
