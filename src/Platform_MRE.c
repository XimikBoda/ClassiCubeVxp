#include "Core.h"
#if defined CC_BUILD_MRE
#include "_PlatformBase.h"
#include "Utils.h"
#include "Funcs.h"
#include <vmsys.h>
#include <vmio.h>
#include <vmchset.h>
#include <vmstdlib.h>
#include <vmtimer.h>
#include <thread.h>

const cc_result ReturnCode_FileShareViolation = 1000000000; /* TODO: not used apparently */
const cc_result ReturnCode_FileNotFound = VM_FILE_OPEN_ERROR;
const cc_result ReturnCode_DirectoryExists = 0;
const cc_result ReturnCode_SocketInProgess = -1;
const cc_result ReturnCode_SocketWouldBlock = -1;
const cc_result ReturnCode_SocketDropped = -1;

const char* Platform_AppNameSuffix = "VXP";
cc_bool Platform_ReadonlyFilesystem = false;
cc_bool Platform_SingleProcess = true;

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
#ifdef _WIN32 //MoDis
	ret = write(fileno(stdout), msg, len);
	ret = write(fileno(stdout), "\n", 1);
	//printf("%s\n", msg);
#endif
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
	char temp[FILENAME_SIZE*2] = "e:\\ClassiCube\\";
	char *tempn = temp + strlen(temp);

	String_EncodeUtf8(tempn, path); //TODO

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


/*########################################################################################################################*
*--------------------------------------------------------Threading--------------------------------------------------------*
*#########################################################################################################################*/
void Thread_MRE_timer(int timer_id) {
	vm_delete_timer(timer_id);
	
	thread_next();
}

void Thread_Sleep(cc_uint32 milliseconds) {
	vm_create_timer(milliseconds, Thread_MRE_timer);

	thread_next();
}

void Thread_Run(void** handle, Thread_StartFunc func, int stackSize, const char* name) {
	*handle = NULL;
}

void Thread_Detach(void* handle) {
}

void Thread_Join(void* handle) {
}

void* Mutex_Create(const char* name) {
	return NULL;
}

void Mutex_Free(void* handle) {
}

void Mutex_Lock(void* handle) {
}

void Mutex_Unlock(void* handle) {
}

void* Waitable_Create(const char* name) {
	return NULL;
}

void Waitable_Free(void* handle) {
}

void Waitable_Signal(void* handle) {
}

void Waitable_Wait(void* handle) {
}

void Waitable_WaitFor(void* handle, cc_uint32 milliseconds) {
}


/*########################################################################################################################*
*---------------------------------------------------------Socket----------------------------------------------------------*
*#########################################################################################################################*/
cc_result Socket_ParseAddress(const cc_string* address, int port, cc_sockaddr* addrs, int* numValidAddrs) {
	return ERR_NOT_SUPPORTED; //TODO
}

cc_result Socket_Create(cc_socket* s, cc_sockaddr* addr, cc_bool nonblocking) {
	return ERR_NOT_SUPPORTED;
}

cc_result Socket_Connect(cc_socket s, cc_sockaddr* addr) {
	return ERR_NOT_SUPPORTED;
}

cc_result Socket_Read(cc_socket s, cc_uint8* data, cc_uint32 count, cc_uint32* modified) {
	return ERR_NOT_SUPPORTED;
}

cc_result Socket_Write(cc_socket s, const cc_uint8* data, cc_uint32 count, cc_uint32* modified) {
	return ERR_NOT_SUPPORTED;
}

void Socket_Close(cc_socket s) {
}

cc_result Socket_CheckReadable(cc_socket s, cc_bool* readable) {
	return ERR_NOT_SUPPORTED;
}

cc_result Socket_CheckWritable(cc_socket s, cc_bool* writable) {
	return ERR_NOT_SUPPORTED;
}


/*########################################################################################################################*
*-----------------------------------------------------Process/Module------------------------------------------------------*
*#########################################################################################################################*/
cc_bool Process_OpenSupported = false;

cc_result Process_StartGame2(const cc_string* args, int numArgs) {
	return SetGameArgs(args, numArgs);
}

void Process_Exit(cc_result code) { 
	vm_exit_app();
	thread_next();
}

cc_result Process_StartOpen(const cc_string* args) {
	return ERR_NOT_SUPPORTED;
}

/*########################################################################################################################*
*--------------------------------------------------------Updater----------------------------------------------------------*
*#########################################################################################################################*/
cc_bool Updater_Supported = false;

cc_bool Updater_Clean(void) { return true; }

const struct UpdaterInfo Updater_Info = { "&eCompile latest source code to update", 0 };

cc_result Updater_Start(const char** action) {
	return ERR_NOT_SUPPORTED;
}

cc_result Updater_GetBuildTime(cc_uint64* timestamp) {
	return ERR_NOT_SUPPORTED;
}

cc_result Updater_MarkExecutable(void) {
	return ERR_NOT_SUPPORTED;
}

cc_result Updater_SetNewBuildTime(cc_uint64 timestamp) {
	return ERR_NOT_SUPPORTED;
}


/*########################################################################################################################*
*-------------------------------------------------------Dynamic lib-------------------------------------------------------*
*#########################################################################################################################*/
const cc_string DynamicLib_Ext = String_FromConst(".dll");

void* DynamicLib_Load2(const cc_string* path) {
	return NULL;
}

void* DynamicLib_Get2(void* lib, const char* name) {
	return NULL;
}

cc_bool DynamicLib_DescribeError(cc_string* dst) {
	return false;
}


/*########################################################################################################################*
*--------------------------------------------------------Platform---------------------------------------------------------*
*#########################################################################################################################*/
void MRE_handle_sysevt(VMINT message, VMINT param) {
#ifdef		SUPPORT_BG
	switch (message) {
	case VM_MSG_CREATE:
		break;
	case VM_MSG_PAINT:
		break;
	case VM_MSG_HIDE:	
		break;
	case VM_MSG_QUIT:
		break;
	}
#else
	switch (message) {
	case VM_MSG_CREATE:
	case VM_MSG_ACTIVE:
		break;
	case VM_MSG_PAINT:
		break;
	case VM_MSG_INACTIVE:
		break;	
	case VM_MSG_QUIT:
		break;	
	}
#endif
}

void Platform_Init(void) {
	VMWCHAR temp_wstr[FILENAME_SIZE];
	vm_ascii_to_ucs2(temp_wstr, FILENAME_SIZE, "e:\\ClassiCube");
	vm_file_mkdir(temp_wstr);

	vm_reg_sysevt_callback(MRE_handle_sysevt);
}

void Platform_Free(void) {}

cc_bool Platform_DescribeError(cc_result res, cc_string* dst) {
	return false;
}

void Platform_ShareScreenshot(const cc_string* filename) {}


/*########################################################################################################################*
*-------------------------------------------------------Encryption--------------------------------------------------------*
*#########################################################################################################################*/
#define MACHINE_KEY "MRE_MRE_MRE_MRE_"

static cc_result GetMachineID(cc_uint32* key) {
	Mem_Copy(key, MACHINE_KEY, sizeof(MACHINE_KEY) - 1);
	return 0;
}


/*########################################################################################################################*
*-------------------------------------------------------Encryption--------------------------------------------------------*
*#########################################################################################################################*/
cc_result Platform_Encrypt(const void* data, int len, cc_string* dst) {
	return ERR_NOT_SUPPORTED;
}

cc_result Platform_Decrypt(const void* data, int len, cc_string* dst) {
	return ERR_NOT_SUPPORTED;
}

cc_result Platform_GetEntropy(void* data, int len) {
	return ERR_NOT_SUPPORTED;
}


/*########################################################################################################################*
*-----------------------------------------------------Configuration-------------------------------------------------------*
*#########################################################################################################################*/
int Platform_GetCommandLineArgs(int argc, STRING_REF char** argv, cc_string* args) {
	int i, count;
	argc--; argv++; /* skip executable path argument */
	if (gameHasArgs) return GetGameArgs(args);

	count = min(argc, GAME_MAX_CMDARGS);
	for (i = 0; i < count; i++)
	{
		args[i] = String_FromReadonly(argv[i]);
	}
	return count;
}

cc_result Platform_SetDefaultCurrentDirectory(int argc, char** argv) {
	return 0;
}

#endif
