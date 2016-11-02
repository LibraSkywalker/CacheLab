#include <stdio.h>
#include "cachelab.h"
#include <string.h>
#include <stdlib.h>

struct array;

struct CacheLine{
    int valid;
    int LRUstate;
    int tag;
};

struct Cache{
    int blockBit;
    int way;
    int setBits;
    int num_hits;
    int num_misses;
    int num_evictions;
    struct CacheLine** line;
} *cache;

struct Cache* new(int setBits,int way,int blockBit) {
    struct Cache* cache = (struct Cache *)malloc(sizeof(struct Cache));

    cache->setBits = setBits;
    cache->way = way;
    cache->blockBit = blockBit;
    int setSize = 1 << cache->setBits;
    cache->line = (struct CacheLine **) malloc(sizeof(struct CacheLine *) * setSize);
    int i,j;
    for (i = 0; i < setSize; ++i) {
        cache->line[i] = (struct CacheLine *) malloc(sizeof(struct CacheLine) * cache->way);
    }
    for (i = 0; i < setSize; ++i)
        for (j = 0; j < cache->way; ++j){
            cache->line[i][j].valid = 0;
            cache->line[i][j].LRUstate = 0;
            cache->line[i][j].tag = 0;
        }
    return cache;
}

void delete(struct Cache* cache) {
    int setSize = 1 << cache->setBits;
    int i;
    for(i = 0; i < setSize; ++i)
    {
        free(cache->line[i]);
    }
    free(cache->line);
    free(cache);
}

FILE* input;
int verbalFlag = 0;

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
        } else if (strcmp(argv[i],"-v") == 0){
            verbalFlag = 1;
        }
    }
    cache = new(setBits,way,blockBit);
}

int persudoLRU(struct Cache* cache, int index) {
    int position = 1;
    while (position < cache->way) {
        int direction = cache->line[index][position].LRUstate;
        cache->line[index][position].LRUstate = 1 - cache->line[index][position].LRUstate;
        position = 2 * position + direction;
    }
    return position - cache->way;
}

void persudoUpdate(struct Cache* cache,int index,int position){
	position += cache->way;
	while (position > 1){
		position /= 2;
        cache->line[index][position].LRUstate = 1 - cache->line[index][position].LRUstate;
	}
}

void update(struct Cache* cache,int index,int position){
	int i;	
	for (i = 0; i < cache->way; i++)
		cache->line[index][i].LRUstate--;
    cache->line[index][position].LRUstate = cache->way;
}

int trueLRU(struct Cache* cache, int index) {
	int i,position = 0;
	for (i = 0; i < cache->way; i++){
		if (cache->line[index][i].LRUstate < cache->line[index][position].LRUstate)
			position = i;
        cache->line[index][i].LRUstate--;
	}
    cache->line[index][position].LRUstate = cache->way;
    return position;
}

void tryAccess(struct Cache* cache, long long position) {
    int index = (int) ((position >> cache->blockBit) & ((1LL << cache->setBits) - 1));
    int tag = (int) (position >> (cache->blockBit + cache->setBits));
	//printf("position = %d index = %d tag = %d\n",position,index,tag);
    for (int i = 0; i < cache->way; i++){
        if (cache->line[index][i].tag == tag){
            cache->num_hits ++;
			update(cache,index,i);
			printf("hit ");
            return;
        }
    }
    // miss
    cache->num_misses++;
	printf("miss ");
    int way = trueLRU(cache,index);
    if (cache->line[index][way].valid){
        cache->num_evictions++;	
		printf("eviction ");

	}
	cache->line[index][way].tag = tag;
    cache->line[index][way].valid = 1;
}

void simulate(FILE * input) {
    char command;
    while (fscanf(input,"%c",&command) != EOF){
        while (command <= ' ' && fscanf(input,"%c",&command) != EOF);
		if(feof(input)) return;
        long long position;
		int length;
        fscanf(input,"%llx,%d",&position,&length);
        if (verbalFlag && command != 'I') printf("%c %llx,%d ",command,position,length);
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
        if (verbalFlag && command != 'I') puts("");
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
