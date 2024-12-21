#include "Core.h"
#if defined CC_BUILD_MRE

#include <vmsys.h>

/*########################################################################################################################*
*---------------------------------------------------------Memory----------------------------------------------------------*
*#########################################################################################################################*/
void* Mem_Set(void* dst, cc_uint8 value, unsigned numBytes) { return memset(dst, value, numBytes); }
void* Mem_Copy(void* dst, const void* src, unsigned numBytes) { return memcpy(dst, src, numBytes); }
void* Mem_Move(void* dst, const void* src, unsigned numBytes) { return memmove(dst, src, numBytes); }

void* Mem_TryAlloc(cc_uint32 numElems, cc_uint32 elemsSize) {
	cc_uint32 size = CalcMemSize(numElems, elemsSize);
	return size ? vm_malloc(size) : NULL;
}

void* Mem_TryAllocCleared(cc_uint32 numElems, cc_uint32 elemsSize) {
	cc_uint32 size = CalcMemSize(numElems, elemsSize);
	return size ? vm_calloc(size) : NULL;
}

void* Mem_TryRealloc(void* mem, cc_uint32 numElems, cc_uint32 elemsSize) {
	cc_uint32 size = CalcMemSize(numElems, elemsSize);
	return size ? vm_realloc(mem, size) : NULL;
}

void Mem_Free(void* mem) {
	if (mem) vm_free(mem);
}

#endif
