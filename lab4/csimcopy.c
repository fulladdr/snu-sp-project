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
  size_t setcount;  
  size_t linecount;
};


struct _cache cache;

int main(int argc, char *argv[]) {
	int opt;
	bool VERBOSE = false;
	while ((opt = getopt(argc, argv, "s:E;b:t:"))!=-1){
		switch (opt){
			case 'v':
				VERBOSE = true;
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
	FILE* tracaefile = fopen(t, "r");
	if (!tracefile){
		fprintf(stdout, "Error opening tracefile\n");
		return -1;
	}
	cache.setcount = 2<<s;
	cache.linecount = E;

	if (!s || !E || !b || !t )
		return -1;

	cache.sets = malloc(sizeof(struct _set) * cache.setcount);
	for (int i=0;i<cache.setcount;i++){
		cache.sets[i].lines = calloc(sizeof(struct _line), cache.linecount);
	}

	char line[32];
	char *tokenoperation, *tokenaddress, *tokensize;
	unsigned int hitcount=0, misscount=0, evictount=0;
	unsigned int address, tag, setindex;

	while (fgets(line, 32, tracefile)){
		if (line[0] == 'I') 
			continue; //ignore instruction load
		tokenoperation = strtok(line, " ,");
		tokenaddress = strtok(NULL, " ,");
		tokensize = strtok(NULL " ,");
		
		if (VERBOSE)
			printf("%s %s,"s ", tokenoperation, tokenaddress, tokensize);

		address = strtol(tokenaddress, NULL, 16);
		tag = (addr >> s)>>b;
		setindex = ((addr << (64-s-b))>>(64-s));

		struct _set *set = &cache.sets[setindex];
		
		bool HIT = false;
		for (size_t i=0;i<cache.linecount;i++){
			struct _line* line = &set->lines[i];
			if (!(line->valid))
				continue;
			if (line->tag != tag)
				continue;
			HIT = true;
			hitcount++;
			update(set, i);
			break;
		}
		
		bool EVICTION = true;
		if (!HIT){
			misscount++;
			for (size_t i=0;i<cache.linecount;i++){
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
				evictioncount++;
				unsigned int lrucount = -1, lruline;
				for (size_t i=0;i<cache.linecount;i++){
					struct _line* line = &set->lines[i];
					if (line->timestamp > lrucount){
						lrucount = line->timestamp;
						lruline = i;
					}
				}
				struct _line *line = &set->lines[lruline];
				line->valid = true;
				line->tag = tag;
				update(set, linecount);
			}
		}
		
	}
}

void update(struct _set *set, size_t linenum){
	for (size_t i=0;i<cache.linecount;i++){
		struct _line *l = &set->lines[i];
		if (i==linenum)
			l->timestamp=0;
		else
			l->timestamp++;
	}
}

