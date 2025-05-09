#include "Page.h"

//MFU algorithm
page *MostFrequentlyUsed(LISTOFPAGES* ListOfPages) {
    page* currentPage = ListOfPages->head;
    page* mfuPage = ListOfPages->head;
    int max = currentPage->counter;

    while(currentPage) 
    {
        if(currentPage->counter > max)
        {
            mfuPage = currentPage;
            max = currentPage->counter;
        }

        currentPage = currentPage->next;
    }

    if (DEBUG==1) 
    {
        printf("EVICTED: p[%03d] c:%02d l:%02f\n", mfuPage->pid, mfuPage->counter, mfuPage->loadTime);
    } 
    
    return mfuPage;
}
