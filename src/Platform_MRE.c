#include "Core.h"
#if defined CC_BUILD_MRE

#include "_PlatformBase.h"
#include "Utils.h"
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

/*########################################################################################################################*
*------------------------------------------------------Logging/Time-------------------------------------------------------*
*#########################################################################################################################*/

void Platform_Log(const char* msg, int len) {
	int ret;
	/* Avoid "ignoring return value of 'write' declared with attribute 'warn_unused_result'" warning */
	//TODO
}

TimeMS DateTime_CurrentUTC(void) {
	VMUINT sec = 0;
	vm_get_utc(&sec);
	return (cc_uint64)sec + UNIX_EPOCH_SECONDS;
}

void DateTime_CurrentLocal(struct cc_datetime* t) {
	vm_time_t time;
	vm_get_time(&time);

	t->year = time.year;
	t->month = time.mon;
	t->day = time.day;
	t->hour = time.hour;
	t->minute = time.min;
	t->second = time.sec;
}

#endif
