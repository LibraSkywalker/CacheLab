#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>

#define LRU_MAX 100000000;

struct cache_line
{
    bool valid;
    int tag;
    int LRUcount;
};

int getSetNum(unsigned ad,int s,int b);
int getTagNum(unsigned ad,int s,int b);
void initCache(struct cache_line **cache,int S,int E);
void loadInstr(struct cache_line *cache,unsigned addr,int snum,int bnum,int E,int *hit,int *miss,int *eviction,bool flag);
void printHelp();

int main(int argc,char **argv)
{
    /*argument for result*/
    int hitSum = 0,missSum = 0,evictSum = 0;
    /*argument*/
    int opt,snum,Enum,bnum = 0;
    char *t = NULL;
    bool hflag = false;
    bool vflag = false;
    int S,E;
    /*argument for trace file*/
    FILE *pFile = NULL;
    char identifier;
    unsigned address;
    int size;
    //looping over arguments
    while (-1 != (opt = getopt(argc,argv,"hvs:E:b:t:")))
    {
        switch(opt)
        {
            case'h':
                hflag = true;
                break;
            case'v':
                vflag = true;
                break;
            case's':
                snum = atoi(optarg);
                break;
            case'E':
                Enum = atoi(optarg);
                break;
            case'b':
                bnum = atoi(optarg);
                break;
            case't':
                t = optarg;
                break;
            default:
                printf("wrong argument!");
                return -1;
        }
    }
    /*help option*/
    if (hflag)
    {
        printHelp();
        return 0;
    }
    /*initiate the cache*/
    struct cache_line *cache = NULL;
    S = 1 << snum;
    E = Enum;
    initCache(&cache,S,E);
    /*open trace files*/
    pFile = fopen(t,"r");
    if (pFile == NULL)
    {
        printf("wrong trace file!\n");
        return -1;
    }
    /*read lines in trace files*/
    while (fscanf(pFile," %c %x,%d",&identifier,&address,&size) > 0)
    {
        if (vflag && identifier != 'I') printf("\n");
        if (identifier == 'L') loadInstr(cache,address,snum,bnum,E,&hitSum,&missSum,&evictSum,vflag);
        else if (identifier == 'S') loadInstr(cache,address,snum,bnum,E,&hitSum,&missSum,&evictSum,vflag);
        else if (identifier == 'M')
        {
            loadInstr(cache,address,snum,bnum,E,&hitSum,&missSum,&evictSum,vflag);
            loadInstr(cache,address,snum,bnum,E,&hitSum,&missSum,&evictSum,vflag);
        }
        else{}
    }
    fclose(pFile);
    free(cache);
    printSummary(hitSum,missSum,evictSum);
    return 0;
}

void initCache(struct cache_line **cache,int S,int E)
{
    *cache = (struct cache_line *) malloc((sizeof(struct cache_line))*S*E);
    if (*cache == NULL)
    {
        printf("not enough space!\n");
        exit(-1);
    }
    int i,j;
    for (i = 0;i < S;++i)
    {
        for (j = 0;j < E;++j)
        {
            (*cache)[i*E+j].valid = false;
            (*cache)[i*E+j].LRUcount = LRU_MAX;
        }
    }
}

int getSetNum(unsigned ad,int s,int b)
{
    int result;
    unsigned mask = 0xffffffff;
    mask = mask << (31 - b - s + 1);
    mask = mask >> (31 - s + 1);
    result = mask & (ad >> b);
    return result;
}

int getTagNum(unsigned ad,int s,int b)
{
    int result;
    unsigned mask = 0xffffffff;
    mask = mask >> (b + s);
    result = mask & (ad >> (b + s));
    return result;
}

void loadInstr(struct cache_line *cache,unsigned address,int snum,int bnum,int E,int *hit,int *miss,int *eviction,bool flag)
{
    int setNum = getSetNum(address,snum,bnum);
    int tagNum = getTagNum(address,snum,bnum);
    int i,j;
    int victim = 0;
     /*check if the data is in set #setNum */
    for (i = 0;i < E;++i)
    {
        /*hit --> update hit,LRUcount*/
        if (cache[setNum * E + i].valid == true && cache[setNum * E + i].tag == tagNum)
        {
            if (flag) printf("hit ");
            *hit = *hit + 1;
            cache[setNum * E + i].LRUcount = LRU_MAX;
            for (j = 0;j < E;++j)
                if (j != i) cache[setNum * E + j].LRUcount = cache[setNum * E + j].LRUcount - 1;
            return;
        }
    }
    /*miss --> update miss*/
    if (flag) printf("miss ");
    *miss = *miss + 1;
    /*find an invalid line in the set to store the data*/
    for (i = 0;i < E;++i)
    {
        /*there is invalid block in set #setNum --> update tag,LRUcount,valid*/
        if (cache[setNum * E + i].valid == false)
        {
            cache[setNum * E + i].valid =true;
            cache[setNum * E + i].tag = tagNum;
            cache[setNum * E + i].LRUcount = LRU_MAX;
            for (j = 0;j < E;++j)
                if (j != i) cache[setNum * E + j].LRUcount = cache[setNum * E + j].LRUcount - 1;
            return;
        }
    }
    /*eviction*/
    *eviction = *eviction + 1;
    /*select a victim --> find the smallest LRUcount,change tag*/
    for (i = 0;i < E;++i)
        if (cache[setNum * E + i].LRUcount < cache[setNum * E + victim].LRUcount) victim = i;

    if (flag) printf("eviction %d ",cache[setNum * E + victim].tag);
    cache[setNum * E + victim].tag = tagNum;
    /*update LRUcount*/
    cache[setNum * E + victim].LRUcount = LRU_MAX;
    for (j = 0;j < E;++j)
        if (j != victim) cache[setNum * E + j].LRUcount = cache[setNum * E + j].LRUcount - 1;
}

void printHelp()
{
    printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n\
    -h        Prints this usage info\n\
    -v        Optional verbose flag \n\
    -s <num>  Number of set index bits (S = 2 s is the number of sets)\n\
    -E <num>  Associativity (number of lines per set)\n\
    -b <num>  Number of block bits (B = 2 b is the block size)\n\
    -t <file> Name of the valgrind trace to replay\n\n\
    For example:\n  \
    linux> ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
}

