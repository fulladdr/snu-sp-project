
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "cachelab.h"

struct _line {
  bool valid;
  int tag;
  int timestamp;
};

struct _set{
  struct _line *lines;
};

struct _cache{
  struct _set *sets;
  unsigned int setcount;  
  unsigned int linecount;
};


struct _cache cache = {};
void update(struct _set *set, unsigned int linenum);

int main(int argc, char *argv[]) {

	int opt;
	bool VERBOSE = false;
	int s, E, b;
	char *t;
	while ((opt = getopt(argc, argv, "s:E:b:t:v"))!=-1){
		switch (opt){
			case 'v':
				VERBOSE = true;
				break;
			case 's':
				s=atoi(optarg);
				break;
			case 'E':
				E=atoi(optarg);
				break;
			case 'b':
				b=atoi(optarg);
				break;
			case 't':
				t=optarg;
				break;
			default :
				exit(EXIT_FAILURE);
		}
	}
	FILE* tracefile = fopen(t, "r");
	if (!tracefile){
		fprintf(stdout, "Error opening tracefile\n");
		return -1;
	}

	cache.setcount = 2 << s;
	cache.linecount = E;

	if (!s || !E || !b || !t ){
		printf("missing arguments\n");
		return -1;
	}

	cache.sets = malloc(sizeof(struct _set) * cache.setcount);
	if (!cache.sets){
		printf("malloc error\n");
		return -1;
	}
	for (int i=0;i<cache.setcount;i++){
		cache.sets[i].lines = calloc(sizeof(struct _line), cache.linecount);
	}
	char line[32];
	char *tokenoperation, *tokenaddress, *tokensize;
	unsigned int hitcount=0, misscount=0, evictioncount=0;
	unsigned int address, tag, setindex;
	unsigned int size;

	while (fgets(line, 32, tracefile) != NULL){
		if (line[0] == 'I')
			continue; //ignore instruction load
		tokenoperation = strtok(line, " ,");
		tokenaddress = strtok(NULL, " ,");
		tokensize = strtok(NULL, " ,");
		size = atoi(tokensize);
		
		if (VERBOSE)
			printf("%s %s,%d ", tokenoperation, tokenaddress, size);

		address = strtol(tokenaddress, NULL, 16);
		tag = (address >> s) >> b;
		setindex = ((address << (64 -s -b)) >> (64 - s));

		struct _set *set = &cache.sets[setindex];
		unsigned int mcount = 0;
		do{
			mcount++;
			bool HIT = false;
			for (unsigned int i=0;i<cache.linecount;i++){
				struct _line* line = &set->lines[i];
				if (!(line->valid))
					continue;
				if (line->tag != tag)
					continue;
				HIT = true;
				++hitcount;
				if (VERBOSE)
					printf("hit ");
				update(set, i);
				break;
			}
			
			bool EVICTION = true;
			if (!HIT){
				++misscount;
				if (VERBOSE)
					printf("miss ");
				for (unsigned int i=0;i<cache.linecount;i++){
					struct _line *line = &set->lines[i];
					if (line->valid)
						continue;
					// empty line found
					EVICTION = false;
					line->valid = true;
					line->tag = tag;
					update(set, i);
					break;
				}
				if (EVICTION){
					++evictioncount;
					if (VERBOSE)
						printf("evict ");
					unsigned int lrucount = -1, lruline;
					for (unsigned int i=0;i<cache.linecount;i++){
						struct _line* line = &set->lines[i];
						if ((line->timestamp) > lrucount){
							lrucount = line->timestamp;
							lruline = i;
						}
					}
					struct _line *line = &set->lines[lruline];
					line->valid = true;
					line->tag = tag;
					update(set, lruline);
				}
			}
		} while(line[1] == 'M' && mcount <2);
		if (VERBOSE)
			printf("\n");
	}
	printSummary(hitcount, misscount, evictioncount);
	
	fclose(tracefile);
	for (unsigned int i=0;i<cache.setcount;i++)
		free(cache.sets[i].lines);
	free(cache.sets);
	return 0;
}

void update(struct _set *set, unsigned int linenum){
	for (unsigned int i=0;i<cache.linecount;i++){
		struct _line *l = &set->lines[i];
		if (i==linenum)
			l->timestamp=0;
		else
			l->timestamp++;
	}
}

