#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include<string.h>
//create CacheBlock object
struct CacheBlock{
	unsigned int valid; //valid bit
	unsigned long int tag; //tag bit
	unsigned int time;
};

int checkIfPowerOfTwo(int);
int checkIfOdd(int);
int searchCache(struct CacheBlock***, unsigned long long int, int, int, int);
void writeToCache(struct CacheBlock***, unsigned long long int, int, int, int); 
unsigned int findMinTime(struct CacheBlock***, int, int);
unsigned int findMaxTime(struct CacheBlock***, int, int);

unsigned int findMaxTime(struct CacheBlock*** cache, int setNumber, int associativity){
	unsigned int maxTime = 0;
	//unsigned int maxTime = 0
	for(int i = 0; i < associativity; i++){
		if(cache[setNumber][i]->time > maxTime){
			maxTime = cache[setNumber][i]->time;
		}
	}
	return maxTime;
}

unsigned int findMinTime(struct CacheBlock*** cache, int setNumber, int associativity){
	unsigned int minTime = 100000000;//1000000000
	int i;
	int index;
	for(i = 0; i < associativity; i++){
		if(cache[setNumber][i]->time < minTime){
			index = i;
			minTime = cache[setNumber][i]->time;
			
		}
	}
	return index; //returns index of block with minimum time
}


int searchCache(struct CacheBlock*** cache, unsigned long long int address, int numberOfSets, int blockSize, int associativity){
		
	int result=0;
	//extract tag bit and set index
	int numBlockBits = log2(blockSize);
	int numSetBits = log2(numberOfSets);
	//int numSetBits = numberOfSets;

	int Mask = (1 << numSetBits)-1;
	
	unsigned int setIndex = (address >> numBlockBits) & Mask;
	unsigned long tag = (address >> numBlockBits) >> numSetBits;

	//then search for hit or miss
	// if hit, result = 1
	// if miss, result = 0

	for(int i = 0; i < associativity; i++){
		if(cache[setIndex][i]->valid == 1){
			if(cache[setIndex][i]->tag == tag){
				result = 1;
			}
		}
	}
	return result;
}

void writeToCache(struct CacheBlock*** cache, unsigned long long int address, int numberOfSets, int blockSize, int associativity){
	//extract tag bit and set index	
	int numBlockBits = log2(blockSize);
	int numSetBits = log2(numberOfSets);
	//int numSetBits = numberOfSets;

	int Mask = (1 << numSetBits)-1;
	
	unsigned int setIndex = (address >> numBlockBits) & Mask;
	unsigned long tag = (address >> numBlockBits) >> numSetBits;
	//printf("assigned setIndex, tag, mask in WriteToCache\n");
	int maxTime = findMaxTime(cache, setIndex, associativity);
	//printf("maxTime: %d\n", maxTime);
	int boolean = 0;
	for(int i = 0; i < associativity; i ++){
		//printf("HELLO\n");
		if(cache[setIndex][i]->valid != 1){
			cache[setIndex][i]->valid = 1;
			cache[setIndex][i]->tag = tag;
			cache[setIndex][i]->time = maxTime + 1;
			boolean = 1;
			//printf("wrote to cache\n");
			//printf("time of write in : %d\n", cache[setIndex][i]->time);
		}
	}
	//could not find any valid bits, must use fifo
	if(boolean == 0){
		int minTimeIndex = findMinTime(cache, setIndex, associativity);
		cache[setIndex][minTimeIndex]->tag = tag;
		cache[setIndex][minTimeIndex]->valid = 1;
		cache[setIndex][minTimeIndex]->time = maxTime + 1;
		//printf("after break\n");
	}
}

int checkIfOdd(int num){
	int answer;
	if(num % 2 == 1){
		answer = 1; //odd
	} 
	else{
		answer = 0; //even
	}
	return answer;
}

int checkIfPowerOfTwo(int num){
	//divide by two until 1 or odd
	int answer;
	while (num != 1 || checkIfOdd(num) != 1){
		num = num / 2;
	}
	if(num == 1){ //yes, power of 2
		answer = 0;
	}
	else{
		answer = 1; //not a power of 2;
	}
	return answer;
}
int main (int argc, char ** argv){
	//read inputs
	int cacheSize;
	int blockSize;
	if(checkIfPowerOfTwo(*argv[1]) == 0){ 	//total size of the cache in bytes - should be a power of 2
		//char temp = *argv[1];		
		cacheSize = atoi(argv[1]);
		//cacheSize = temp - '0';
	}
	else{
		printf("error\n");
		exit(0);
	}

	//printf("cacheSize: %d\n", cacheSize);
	if(checkIfPowerOfTwo(*argv[2]) == 0){	// size of cache block in bytes - should be a power of 2
		//char temp2 = *argv[2];		
		blockSize = atoi(argv[2]);
		//blockSize = temp2 - '0';
	}
	else{
		printf("error\n");
		exit(0);
	}
	//printf("blockSize : %d\n", blockSize);
	char* associativityString;	//direct : simulate a direct mapped cache - 1
						//assoc : simulate a fully associative cache
						//assoc.n : simulate an n way associative cache - n should be a power of 2
	int associativity;
	//if(*argv[4] == "direct"){
	if(strcmp(argv[4], "direct")==0){
		associativity = 1; //associativity = number of blocks in a set
	}
	//else if(*argv[4] == "assoc"){
	else if(strcmp(argv[4], "assoc")==0){
		associativity = cacheSize/blockSize; //associativity = cache size / number of blocks
	}
	else{
		associativityString = argv[4];
		//printf("associativityString : %s\n", associativityString);
		associativity = associativityString[6]-'0';
	}
	//printf("associativity : %d\n", associativity);

	//char* cachePolicy = argv[3];	// fifo or lru - UNCOMMENT LATER
	//printf("cache policy: %s\n", cachePolicy);
			
	char prefetchChar = *argv[5];			
	int prefetchSize = prefetchChar-'0';	//number of adjacent blocks that should be prefetched by the prefetcher in case of a miss
	//printf("prefetch size : %d\n", prefetchSize);

	
	int numWrites = 0;;	//without prefetching
	int numReads = 0;
	int numMisses = 0;
	int numHits = 0;

	/*int numReadsWP = 0; //with prefetching
	int numWritesWP = 0;
	int numMissesWP = 0;
	int numHitsWP = 0;*/

	int numberOfSets = (cacheSize) / (associativity * blockSize); 
	//int numberOfSets = log2(cacheSize / (associativity*blockSize));
	//int offSetBits = log2(blockSize);
	
	//initialize cache
	struct CacheBlock*** cache = malloc(numberOfSets*sizeof(struct CacheBlock**));
	for(int i = 0; i < numberOfSets; i++){
		cache[i] = malloc(associativity*sizeof(struct CacheBlock*));
		for(int j = 0; j < associativity; j++){
			cache[i][j] = malloc(sizeof(struct CacheBlock));
		}
	}

	struct CacheBlock*** cache2 = malloc(numberOfSets*sizeof(struct CacheBlock**));
	for(int i = 0; i < numberOfSets; i++){
		cache2[i] = malloc(associativity*sizeof(struct CacheBlock*));
		for(int j = 0; j < associativity; j++){
			cache2[i][j] = malloc(sizeof(struct CacheBlock));
		}
	}
	
	//CALCULATE INDEX, TAG, AND OFFSET BITS!!!!!!
	
	//read file
	FILE *fp = fopen(argv[6], "r");
	if(fp == NULL){
		printf("error\n");
		exit(0);
	}
	unsigned long long int address;
	char command;
	while(fscanf(fp, "%c %llx\n", &command, &address)>0){
		//printf("%c %llx\n", command, address);
		if(command == '#'){
			break;
		}
		if(command == 'W'){
			numWrites+=1;
			//printf("Incremented W\n");
		}
		if(searchCache(cache, address, numberOfSets, blockSize, associativity) == 1){	//hit
			numHits++;
			//printf("Hit\n");
		}
		else if(searchCache(cache, address, numberOfSets, blockSize, associativity) == 0){ 		//miss
			numMisses++;
			numReads++;
			//printf("miss\n");
			writeToCache(cache, address, numberOfSets, blockSize, associativity);
		}
	}
	fclose(fp);

	int numReadsWP = 0;
	int numMissesWP = 0;
	int numHitsWP = 0;
	FILE* secondRead = fopen(argv[6], "r");
	unsigned long long int address2;
	char command2;
	while(fscanf(secondRead, "%c %llx\n", &command2, &address2)>0){
		//printf("%c %llx\n", command, address);
		if(command2 == '#'){
			break;
		}
		if(searchCache(cache2, address2, numberOfSets, blockSize, associativity) == 1){	//hit
			numHitsWP++;
			//printf("Hit\n");
		}
		else if(searchCache(cache2, address2, numberOfSets, blockSize, associativity) == 0){ 		//miss
			numMissesWP++;
			numReadsWP++;
			//printf("miss\n");
			unsigned long long int prefetchAddress = address2;
			for(int i = 0; i < prefetchSize; i++){
				prefetchAddress = prefetchAddress+blockSize;
				if(searchCache(cache2, prefetchAddress, numberOfSets, blockSize, associativity)==0){
					writeToCache(cache2, prefetchAddress, numberOfSets, blockSize, associativity);
					numReadsWP++;
				}
			}
			writeToCache(cache2, address2, numberOfSets, blockSize, associativity);
		}
	}
	fclose(secondRead);

	printf("no-prefetch\n");
	printf("Memory reads : %d\n", numReads); //WOP = without prefetching
	printf("Memory writes : %d\n", numWrites);
	printf("Cache hits : %d\n", numHits);
	printf("Cache misses : %d\n", numMisses);
	printf("with-prefetch\n");
	printf("Memory reads : %d\n",numReadsWP); //WP = with prefetching
	printf("Memory writes : %d\n", numWrites);
	printf("Cache hits : %d\n", numHitsWP);
	printf("Cache misses : %d\n", numMissesWP);
	return 0;
}