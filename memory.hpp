#ifndef MEMORY_HPP
#define MEMORY_HPP

#ifdef NDEBUG
#	define DISABLE_DEBUG_MEMORY
#endif

#define DISABLE_DEBUG_MEMORY

#define __stringify(x) __stringify2(x)
#define __stringify2(x) #x
#define LINESTR __stringify(__LINE__)
#define LOCSTR __FILE__ ": " __stringify(__LINE__) " (" __FUNCTION__ ")"

#ifdef DISABLE_DEBUG_MEMORY
	#define NEW new
	#define ALLOCED(x) x
#else
	#define NEW new(LOCSTR)
	#define ALLOCED(x) __alloced(x, LOCSTR)
#endif

void unrecordAlloc(void *mem);
void recordAlloc(void *mem, const char *location);
void dumpAllocs();

template<class T>
T *__alloced(T *mem, const char *location) {
	recordAlloc(mem, location);
	return mem;
}

void *__markedAlloc(size_t size, const char *location);

#ifndef DISABLE_DEBUG_MEMORY

inline void *operator new(size_t size, const char *location) {
	return __markedAlloc(size, location);
}
inline void *operator new[](size_t size, const char *location) {
	return __markedAlloc(size, location);
}
inline void operator delete(void *mem) {
	if(!mem) return;
	unrecordAlloc(mem);
	free(mem);
}
inline void operator delete[](void *mem) {
	if(!mem) return;
	unrecordAlloc(mem);
	free(mem);
}
inline void operator delete(void *mem, const char *location) {
	if(!mem) return;
	unrecordAlloc(mem);
	free(mem);
}
inline void operator delete[](void *mem, const char *location) {
	if(!mem) return;
	unrecordAlloc(mem);
	free(mem);
}

#endif

#endif