#pragma once

#include "../../ThirdParty/phnt/phnt.h"

BOOLEAN NtApiHook(PVOID functionToHook, PVOID targetFunction);
BOOLEAN NtApiBreakAndHook(PVOID functionToHook, PVOID targetFunction);

extern "C" NTSTATUS NTAPI HookedNtAllocateVirtualMemory(_In_ HANDLE ProcessHandle, _Inout_ _At_(*BaseAddress, _Readable_bytes_(*RegionSize) _Writable_bytes_(*RegionSize) _Post_readable_byte_size_(*RegionSize)) PVOID * BaseAddress, _In_ ULONG_PTR ZeroBits, _Inout_ PSIZE_T RegionSize, _In_ ULONG AllocationType, _In_ ULONG Protect);
extern "C" NTSTATUS NTAPI HookedNtProtectVirtualMemory(_In_ HANDLE ProcessHandle, _Inout_ PVOID * BaseAddress, _Inout_ PSIZE_T RegionSize, _In_ ULONG NewProtect, _Out_ PULONG OldProtect);
extern "C" NTSTATUS NTAPI HookedNtCreateSection(_Out_ PHANDLE SectionHandle, _In_ ACCESS_MASK DesiredAccess, _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes, _In_opt_ PLARGE_INTEGER MaximumSize, _In_ ULONG SectionPageProtection, _In_ ULONG AllocationAttributes, _In_opt_ HANDLE FileHandle);
extern "C" LONG CALLBACK VectoredHandler(_In_ PEXCEPTION_POINTERS ExceptionInfo);

extern "C" void _SetContextNtAllocateVirtualMemoryAddress(void* address);
extern "C" void _SaveContextNtAllocateVirtualMemory();
extern "C" DWORD64 _QueryContextNtAllocateVirtualMemoryCaller();
extern "C" DWORD64 _QueryContextNtAllocateVirtualMemoryRip();
extern "C" DWORD64 _QueryContextNtAllocateVirtualMemoryRbp();
extern "C" DWORD64 _QueryContextNtAllocateVirtualMemoryRsp();
extern "C" void _ReleaseNtAllocateVirtualMemoryContextLock();

extern "C" void _SetContextNtProtectVirtualMemoryAddress(void* address);
extern "C" void _SaveContextNtProtectVirtualMemory();
extern "C" DWORD64 _QueryContextNtProtectVirtualMemoryCaller();
extern "C" DWORD64 _QueryContextNtProtectVirtualMemoryRip();
extern "C" DWORD64 _QueryContextNtProtectVirtualMemoryRbp();
extern "C" DWORD64 _QueryContextNtProtectVirtualMemoryRsp();
extern "C" void _ReleaseNtProtectVirtualMemoryContextLock();