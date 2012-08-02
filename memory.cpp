#include <map>
#include <cstdlib>

typedef std::map<const char*, int> countPool;
typedef std::map<void*, const char*> unmarkedPool;
countPool *allocCounts;
unmarkedPool *unmarkedAllocs;

struct MemoryManagerInitializer {
	MemoryManagerInitializer() {
		allocCounts = new countPool();
		unmarkedAllocs = new unmarkedPool();
	}
};
static MemoryManagerInitializer *memInit;

void recordAlloc(void *mem, const char *location)
{
	if(!memInit)
		memInit = new MemoryManagerInitializer;
	
	(*unmarkedAllocs)[mem] = location;
	(*allocCounts)[location]++;
}

void unrecordAlloc(void *mem)
{
	if(!memInit)
		memInit = new MemoryManagerInitializer;
	
	const char *str = *(((const char**)mem) - 1);
	if(allocCounts->find(str) != allocCounts->end())
		(*allocCounts)[str]--;
	else {
		if(unmarkedAllocs->find(mem) == unmarkedAllocs->end())
			return;
		
		(*allocCounts)[(*unmarkedAllocs)[mem]]--;
		unmarkedAllocs->erase(mem);
	}
}

void *__markedAlloc(size_t size, const char *location)
{
	if(!memInit)
		memInit = new MemoryManagerInitializer;
	
	void *alloc = malloc(size + sizeof(const char*));
	const char **str = (const char**)alloc;
	void *ret = (void*)(str+1);
	
	*str = location;
	
	(*allocCounts)[location]++;
	return ret;
}


void dumpAllocs()
{
	if(!memInit)
		return;
	
	FILE *fout = fopen("memory.log", "w");
	for(countPool::iterator ii=allocCounts->begin(); ii!=allocCounts->end(); ii++)
	{
		if(ii->second == 0)
			continue;
		fprintf(fout, "%3i %s\n", (int)ii->second, (const char*)ii->first);
	}
	fclose(fout);
}
