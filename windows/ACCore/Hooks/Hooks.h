#ifndef _HOOKS_H_
#define _HOOKS_H_

#include "../../ThirdParty/phnt/phnt.h"

BOOLEAN NtApiHook(PVOID functionToHook, PVOID targetFunction);
BOOLEAN NtApiBreakAndHook(PVOID functionToHook, PVOID targetFunction);

extern "C" {
NTSTATUS NTAPI HookedNtAllocateVirtualMemory(
    _In_ HANDLE ProcessHandle,
    _Inout_ _At_(*BaseAddress,
                 _Readable_bytes_(*RegionSize) _Writable_bytes_(*RegionSize)
                     _Post_readable_byte_size_(*RegionSize)) PVOID* BaseAddress,
    _In_ ULONG_PTR ZeroBits, _Inout_ PSIZE_T RegionSize,
    _In_ ULONG AllocationType, _In_ ULONG Protect);
NTSTATUS NTAPI HookedNtProtectVirtualMemory(_In_ HANDLE ProcessHandle,
                                            _Inout_ PVOID* BaseAddress,
                                            _Inout_ PSIZE_T RegionSize,
                                            _In_ ULONG NewProtect,
                                            _Out_ PULONG OldProtect);
NTSTATUS NTAPI HookedNtCreateSection(
    _Out_ PHANDLE SectionHandle, _In_ ACCESS_MASK DesiredAccess,
    _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes,
    _In_opt_ PLARGE_INTEGER MaximumSize, _In_ ULONG SectionPageProtection,
    _In_ ULONG AllocationAttributes, _In_opt_ HANDLE FileHandle);
LONG CALLBACK VectoredHandler(_In_ PEXCEPTION_POINTERS ExceptionInfo);

void _SetContextNtAllocateVirtualMemoryAddress(void* address);
void _SaveContextNtAllocateVirtualMemory();
DWORD64 _QueryContextNtAllocateVirtualMemoryCaller();
DWORD64 _QueryContextNtAllocateVirtualMemoryRip();
DWORD64 _QueryContextNtAllocateVirtualMemoryRbp();
DWORD64 _QueryContextNtAllocateVirtualMemoryRsp();
void _ReleaseNtAllocateVirtualMemoryContextLock();

void _SetContextNtProtectVirtualMemoryAddress(void* address);
void _SaveContextNtProtectVirtualMemory();
DWORD64 _QueryContextNtProtectVirtualMemoryCaller();
DWORD64 _QueryContextNtProtectVirtualMemoryRip();
DWORD64 _QueryContextNtProtectVirtualMemoryRbp();
DWORD64 _QueryContextNtProtectVirtualMemoryRsp();
void _ReleaseNtProtectVirtualMemoryContextLock();
};
#endif