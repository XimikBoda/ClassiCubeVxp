#include "Core.h"
#if defined CC_BUILD_MRE

#include "_PlatformBase.h"
#include "Utils.h"
#include <vmsys.h>
#include <vmio.h>
#include <vmchset.h>
#include <vmstdlib.h>


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


/*########################################################################################################################*
*-------------------------------------------------------Crash handling----------------------------------------------------*
*#########################################################################################################################*/
void CrashHandler_Install(void) {}

void Process_Abort2(cc_result result, const char* raw_msg) {
	Logger_DoAbort(result, raw_msg, NULL);
}


/*########################################################################################################################*
*--------------------------------------------------------Stopwatch--------------------------------------------------------*
*#########################################################################################################################*/
#define MS_PER_SEC 1000ULL

cc_uint64 Stopwatch_Measure(void) {
	return (cc_uint64)vm_get_tick_count() * MS_PER_SEC;
}

cc_uint64 Stopwatch_ElapsedMicroseconds(cc_uint64 beg, cc_uint64 end) {
	if (end < beg) return 0;
	return end - beg;
}


/*########################################################################################################################*
*-----------------------------------------------------Directory/File------------------------------------------------------*
*#########################################################################################################################*/
void Platform_EncodePath(cc_filepath* dst, const cc_string* path) {
	VMWCHAR* wstr = dst->buffer;
	char temp[FILENAME_SIZE];

	String_EncodeUtf8(temp, path); //TODO
	vm_ascii_to_ucs2(wstr, FILENAME_SIZE, temp);
}


void Directory_GetCachePath(cc_string* path) {}

cc_result Directory_Create(const cc_filepath* path) {
	/* read/write/search permissions for owner and group, and with read/search permissions for others. */
	/* TODO: Is the default mode in all cases */

	return vm_file_mkdir((VMWSTR)path->buffer);
}

int File_Exists(const cc_filepath* path) {
	return vm_file_get_attributes((VMWSTR)path->buffer) != -1;
}

cc_result Directory_Enum(const cc_string* dirPath, void* obj, Directory_EnumCallback callback) {
	cc_string path; char pathBuffer[FILENAME_SIZE];
	cc_filepath str;
	vm_fileinfo_ext dir;
	char src[FILENAME_SIZE];
	int len, res, is_dir;
	VMWCHAR find_req[5] = {'\\', '*', '.', '*', '\0'};

	Platform_EncodePath(&str, dirPath);
	vm_wstrcat((VMWSTR)str.buffer, find_req);

	int find_h = vm_find_first_ext((VMWSTR)str.buffer, &dir);

	if (find_h < 0) return find_h;

	/* POSIX docs: "When the end of the directory is encountered, a null pointer is returned and errno is not changed." */
	/* errno is sometimes leftover from previous calls, so always reset it before readdir gets called */
	String_InitArray(path, pathBuffer);

	do {
		path.length = 0;
		String_Format1(&path, "%s/", dirPath);

		vm_ucs2_to_ascii(src, FILENAME_SIZE, dir.filefullname);

		/* ignore . and .. entry */
		if (src[0] == '.' && src[1] == '\0') continue;
		if (src[0] == '.' && src[1] == '.' && src[2] == '\0') continue;

		len = String_Length(src);
		String_AppendUtf8(&path, src, len);


		is_dir = !!(dir.attributes & VM_FS_ATTR_DIR);
		/* TODO: fallback to stat when this fails */

		callback(&path, obj, is_dir);
	} while (vm_find_next_ext(find_h, &dir) == 0);

	vm_find_close_ext(find_h);
	
	return 0;
}

static cc_result File_Do(cc_file* file, VMWSTR path, VMUINT mode) {
	*file = vm_file_open(path, mode, VM_TRUE);
	return *file < 0 ? *file : 0;
}

cc_result File_Open(cc_file* file, const cc_filepath* path) {
	return File_Do(file, (VMWSTR)path->buffer, MODE_READ);
}
cc_result File_Create(cc_file* file, const cc_filepath* path) {
	return File_Do(file, (VMWSTR)path->buffer, MODE_CREATE_ALWAYS_WRITE);
}
cc_result File_OpenOrCreate(cc_file* file, const cc_filepath* path) {
	if(File_Exists(path))
		return File_Do(file, (VMWSTR)path->buffer, MODE_CREATE_ALWAYS_WRITE);
	else
		return File_Do(file, (VMWSTR)path->buffer, MODE_WRITE);
}

cc_result File_Read(cc_file file, void* data, cc_uint32 count, cc_uint32* bytesRead) {
	int res = vm_file_read(file, data, count, bytesRead);
	return res < 0 ? res : 0;
}

cc_result File_Write(cc_file file, const void* data, cc_uint32 count, cc_uint32* bytesWrote) {
	int res = vm_file_write(file, data, count, bytesWrote);
	return res < 0 ? res : 0;
}

cc_result File_Close(cc_file file) {
	vm_file_close(file);
	return 0;
}

cc_result File_Seek(cc_file file, int offset, int seekType) {
	static cc_uint8 modes[3] = { BASE_BEGIN, BASE_CURR, BASE_END };
	int res = vm_file_seek(file, offset, modes[seekType]);
	return res < 0 ? res : 0;
}

cc_result File_Position(cc_file file, cc_uint32* pos) {
	*pos = vm_file_tell(file);
	return 0;
}

cc_result File_Length(cc_file file, cc_uint32* len) {
	int res = vm_file_getfilesize(file, len);
	return res != 0 ? res : 0;
}

#endif
