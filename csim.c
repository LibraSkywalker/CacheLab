#include <stdio.h>
#include "cachelab.h"
#include <string.h>
#include <stdlib.h>

struct array;

const long long UNUSED = -1;

struct Cache{
    int blockBit;
    int way;
    int setBits;
    int num_hits;
    int num_misses;
    int num_evictions;
    long long ** tag;
    int ** LRUstate;
} *cache;

struct Cache* new(int setBits,int way,int blockBit) {
    struct Cache* cache = (struct Cache *)malloc(sizeof(struct Cache));

    cache->setBits = setBits;
    cache->way = way;
    cache->blockBit = blockBit;
    int setSize = 1 << cache->setBits;
    cache->tag = (long long **) malloc(sizeof(int*) * setSize);
    cache->LRUstate = (int **) malloc(sizeof(int*) * setSize);
    int i,j;
    for (i = 0; i < setSize; ++i) {
        cache->tag[i] = (long long *) malloc(sizeof(int) * cache->way);
        cache->LRUstate[i] = (int *) malloc(sizeof(int) * cache->way);
    }
    for (i = 0; i < setSize; ++i)
        for (j = 0; j < cache->way; ++j){
            cache->tag[i][j] = UNUSED;
            cache->LRUstate[i][j] = 0;
        }
    return cache;
}

void delete(struct Cache* cache) {
    int setSize = 1 << cache->setBits;
    int i;
    for(i = 0; i < setSize; ++i)
    {
        free(cache->tag[i]);
        free(cache->LRUstate[i]);
    }
    free(cache->tag);
    free(cache->LRUstate);
    free(cache);
}

FILE* input;

void initialize(int argc,char* argv[]){
    int i;
    int setBits = 0,way = 0,blockBit = 0;
    for (i = 0; i < argc; i++){
        if (strcmp(argv[i],"-s") == 0){
            i++;
            setBits = atoi(argv[i]);
        } else if (strcmp(argv[i],"-E") == 0){
            i++;
            way = atoi(argv[i]);
        } else if (strcmp(argv[i],"-b") == 0){
            i++;
            blockBit = atoi(argv[i]);

        } else if (strcmp(argv[i],"-t") == 0){
            i++;
            input = fopen(argv[i],"r");
        }
    }
    cache = new(setBits,way,blockBit);
}

int persudoLRU(struct Cache* cache, int index) {
    int position = 1;
    while (position < cache->way) {
        int direction = cache->LRUstate[index][position];
        cache->LRUstate[index][position] = 1 - cache->LRUstate[index][position];
        position = 2 * position + direction;
    }
    return position - cache->way;
}

void persudoupdate(struct Cache* cache,int index,int position){
	position += cache->way;
	while (position > 1){
		position /= 2;
		cache->LRUstate[index][position] = 1 - cache->LRUstate[index][position]; 
	}
}

void update(struct Cache* cache,int index,int position){
	int i;	
	for (i = 0; i < cache->way; i++)
		cache->LRUstate[index][i]--;
	cache->LRUstate[index][position] = 4;
}

int trueLRU(struct Cache* cache, int index) {
	int i,position = 0;
	for (i = 0; i < cache->way; i++){
		if (cache->LRUstate[index][i] < cache-> LRUstate[index][position])
			position = i;
		cache->LRUstate[index][i]--;
	}
	cache->LRUstate[index][position] = 4;
	return position;
}

void tryAccess(struct Cache* cache, long long position) {
    int index =(position >> cache->blockBit) & ((1LL << cache->setBits) - 1);
    long long tag = (position >> (cache->blockBit + cache->setBits));
	//printf("position = %d index = %d tag = %d\n",position,index,tag);
    for (int i = 0; i < cache->way; i++){
        if (cache->tag[index][i] == tag){
            cache->num_hits ++;
			update(cache,index,i);
			//printf("hit!");
            return;
        }
    }
    // miss
    cache->num_misses++;
	//printf("miss!");
    int way = trueLRU(cache,index);
    if (cache->tag[index][way] != -1){
        cache->num_evictions++;	
		//printf("eviction!");
	}
	cache->tag[index][way] = tag;
	puts("");
}

void simulate(FILE * input) {
    char command;
    while (fscanf(input,"%c",&command) != EOF){
        while (command <= ' ' && fscanf(input,"%c",&command) != EOF);
		if(feof(input)) return;
        long long position;
		length;
        fscanf(input,"%x,%d",&position,&length);

		//printf("address = %d length = %d\n",position,length);
		        
		switch (command){
            case 'L' :
                tryAccess(cache,position);
				break;
            case 'S' :
                tryAccess(cache,position);
				break;            
			case 'M' :
                tryAccess(cache,position);
                tryAccess(cache,position);
				break;            
			default:;
        }
    }
}

int main(int argc, char* argv[])
{

    initialize(argc,argv);

    simulate(input);

    printSummary(cache->num_hits, cache->num_misses, cache->num_evictions);

    delete(cache);

    return 0;
}
