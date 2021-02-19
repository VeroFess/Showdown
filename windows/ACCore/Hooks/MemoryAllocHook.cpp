#include "../../ThirdParty/phnt/phnt.h"
#include "../Logger.hpp"
#include "../Errors.h"
#include "Hooks.h"
#include <dbghelp.h>

#define PAGE_EXECUTE_FLAGS (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)

extern "C" NTSTATUS NTAPI HookedNtAllocateVirtualMemory(_In_ HANDLE ProcessHandle, _Inout_ _At_(*BaseAddress, _Readable_bytes_(*RegionSize) _Writable_bytes_(*RegionSize) _Post_readable_byte_size_(*RegionSize)) PVOID * BaseAddress, _In_ ULONG_PTR ZeroBits, _Inout_ PSIZE_T RegionSize, _In_ ULONG AllocationType, _In_ ULONG Protect) {
	if (Protect & PAGE_EXECUTE_FLAGS) {
		CONTEXT context = {};
		STACKFRAME64 frame = {};
		SIZE_T returnLength = 0;

		context.ContextFlags = CONTEXT_FULL;
		RtlCaptureContext(&context);
		context.Rip = _QueryContextNtAllocateVirtualMemoryRip();
		context.Rbp = _QueryContextNtAllocateVirtualMemoryRbp();
		context.Rsp = _QueryContextNtAllocateVirtualMemoryRsp();
		frame.AddrPC.Offset = _QueryContextNtAllocateVirtualMemoryRip();
		frame.AddrPC.Mode = AddrModeFlat;
		frame.AddrFrame.Offset = _QueryContextNtAllocateVirtualMemoryRbp();
		frame.AddrFrame.Mode = AddrModeFlat;
		frame.AddrStack.Offset = _QueryContextNtAllocateVirtualMemoryRsp();
		frame.AddrStack.Mode = AddrModeFlat;

		Log(MSG_TRACE_V, "[MCPAC] [Thread] Begin Stack trace for NtAllocateVirtualMemory, try to allocate 0x%016X byte executeable memory.\n", *RegionSize);

		while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, GetCurrentProcess(), GetCurrentThread(), &frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
			auto memoryMappedFilenameInformationBuffer = reinterpret_cast<PUNICODE_STRING>(LoggerMalloc(1024));
			if (memoryMappedFilenameInformationBuffer == nullptr) {
				continue;
			}

			if (NT_SUCCESS(NtQueryVirtualMemory(NtCurrentProcess(), reinterpret_cast<PVOID64>(frame.AddrPC.Offset), MemoryMappedFilenameInformation, memoryMappedFilenameInformationBuffer, 1024, &returnLength))) {
				Log(MSG_TRACE_V, "    The return address <0x%016X> is in module %.*ls.\n", context.Rip, memoryMappedFilenameInformationBuffer->Length, memoryMappedFilenameInformationBuffer->Buffer);
			} else {
				Log(MSG_TRACE_V, "    <0x%016X> NULL\n", frame.AddrPC.Offset);
			}

			LoggerFree(memoryMappedFilenameInformationBuffer);
		}

	}

	_ReleaseNtAllocateVirtualMemoryContextLock();

	auto result = NtAllocateVirtualMemory(ProcessHandle, BaseAddress, ZeroBits, RegionSize, AllocationType, Protect);
	return result;
}

extern "C" NTSTATUS NTAPI HookedNtProtectVirtualMemory(_In_ HANDLE ProcessHandle, _Inout_ PVOID * BaseAddress, _Inout_ PSIZE_T RegionSize, _In_ ULONG NewProtect, _Out_ PULONG OldProtect) {
	if (NewProtect & PAGE_EXECUTE_FLAGS) {
		CONTEXT context = {};
		STACKFRAME64 frame = {};
		SIZE_T returnLength = 0;

		context.ContextFlags = CONTEXT_FULL;
		RtlCaptureContext(&context);
		context.Rip = _QueryContextNtProtectVirtualMemoryRip();
		context.Rbp = _QueryContextNtProtectVirtualMemoryRbp();
		context.Rsp = _QueryContextNtProtectVirtualMemoryRsp();
		frame.AddrPC.Offset = _QueryContextNtProtectVirtualMemoryRip();
		frame.AddrPC.Mode = AddrModeFlat;
		frame.AddrFrame.Offset = _QueryContextNtProtectVirtualMemoryRbp();
		frame.AddrFrame.Mode = AddrModeFlat;
		frame.AddrStack.Offset = _QueryContextNtProtectVirtualMemoryRsp();
		frame.AddrStack.Mode = AddrModeFlat;

		Log(MSG_TRACE_V, "[MCPAC] [Thread] Begin Stack trace for NtProtectVirtualMemory, try to modify 0x%016X(0x%016X byte) memory to executeable\n", *BaseAddress, *RegionSize);

		while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, GetCurrentProcess(), GetCurrentThread(), &frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
			auto memoryMappedFilenameInformationBuffer = reinterpret_cast<PUNICODE_STRING>(LoggerMalloc(1024));
			if (memoryMappedFilenameInformationBuffer == nullptr) {
				continue;
			}

			if (NT_SUCCESS(NtQueryVirtualMemory(NtCurrentProcess(), reinterpret_cast<PVOID64>(frame.AddrPC.Offset), MemoryMappedFilenameInformation, memoryMappedFilenameInformationBuffer, 1024, &returnLength))) {
				Log(MSG_TRACE_V, "    The return address <0x%016X> is in module %.*ls.\n", context.Rip, memoryMappedFilenameInformationBuffer->Length, memoryMappedFilenameInformationBuffer->Buffer);
			} else {
				Log(MSG_TRACE_V, "    <0x%016X> NULL\n", frame.AddrPC.Offset);
			}

			LoggerFree(memoryMappedFilenameInformationBuffer);
		}

	}

	_ReleaseNtProtectVirtualMemoryContextLock();

	return NtProtectVirtualMemory(ProcessHandle, BaseAddress, RegionSize, NewProtect, OldProtect);
}

extern "C" NTSTATUS NTAPI HookedNtCreateSection(_Out_ PHANDLE SectionHandle, _In_ ACCESS_MASK DesiredAccess, _In_opt_ POBJECT_ATTRIBUTES ObjectAttributes, _In_opt_ PLARGE_INTEGER MaximumSize, _In_ ULONG SectionPageProtection, _In_ ULONG AllocationAttributes, _In_opt_ HANDLE FileHandle) {
	return NtCreateSection(SectionHandle, DesiredAccess, ObjectAttributes, MaximumSize, SectionPageProtection, AllocationAttributes, FileHandle);
}

extern "C" LONG CALLBACK VectoredHandler(_In_ PEXCEPTION_POINTERS ExceptionInfo) {
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT) {
		CONTEXT context = {};
		STACKFRAME64 frame = {};
		SIZE_T returnLength = 0;

		memcpy(&context, ExceptionInfo->ContextRecord, sizeof(CONTEXT));
		frame.AddrPC.Offset = context.Rip;
		frame.AddrPC.Mode = AddrModeFlat;
		frame.AddrFrame.Offset = context.Rbp;
		frame.AddrFrame.Mode = AddrModeFlat;
		frame.AddrStack.Offset = context.Rsp;
		frame.AddrStack.Mode = AddrModeFlat;

		Log(MSG_TRACE_V, "[MCPAC] [Thread] Begin Stack trace for api call trace\n");

		while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, GetCurrentProcess(), GetCurrentThread(), &frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {
			auto memoryMappedFilenameInformationBuffer = reinterpret_cast<PUNICODE_STRING>(LoggerMalloc(1024));
			if (memoryMappedFilenameInformationBuffer == nullptr) {
				continue;
			}

			if (NT_SUCCESS(NtQueryVirtualMemory(NtCurrentProcess(), reinterpret_cast<PVOID64>(frame.AddrPC.Offset), MemoryMappedFilenameInformation, memoryMappedFilenameInformationBuffer, 1024, &returnLength))) {
				Log(MSG_TRACE_V, "    The return address <0x%016X> is in module %.*ls.\n", context.Rip, memoryMappedFilenameInformationBuffer->Length, memoryMappedFilenameInformationBuffer->Buffer);
			} else {
				Log(MSG_TRACE_V, "    <0x%016X> NULL\n", frame.AddrPC.Offset);
			}

			LoggerFree(memoryMappedFilenameInformationBuffer);
		}

	#ifdef _WIN64
		ExceptionInfo->ContextRecord->Rip++;
	#else
		ExceptionInfo->ContextRecord->Eip++;
	#endif
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	return EXCEPTION_CONTINUE_SEARCH;
}