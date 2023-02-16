#include <windows.h>
extern "C" {
__declspec(thread) __int64 NtAllocateVirtualMemoryContextReturnAddress = 0;
__declspec(thread) __int64 NtAllocateVirtualMemoryContextFunctionRIP = 0;
__declspec(thread) __int64 NtAllocateVirtualMemoryContextRbp = 0;
__declspec(thread) __int64 NtAllocateVirtualMemoryContextRsp = 0;
__declspec(thread) __int64 NtAllocateVirtualMemoryLock = 0;

__declspec(thread) __int64 NtProtectVirtualMemoryContextReturnAddress = 0;
__declspec(thread) __int64 NtProtectVirtualMemoryContextFunctionRIP = 0;
__declspec(thread) __int64 NtProtectVirtualMemoryContextRbp = 0;
__declspec(thread) __int64 NtProtectVirtualMemoryContextRsp = 0;
__declspec(thread) __int64 NtProtectVirtualMemoryLock = 0;
};