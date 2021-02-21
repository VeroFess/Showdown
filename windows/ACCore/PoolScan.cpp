#include "../ThirdParty/phnt/phnt.h"
#include "Logger.hpp"
#include "Verifier.h"
#include "Errors.h"

BOOL QuerySystemPool() {
	BOOL                       returnValue = TRUE;
	PVOID                      poolTagInformationBuffer = nullptr;
	SIZE_T                     bufferSize = 0x1000;
	ULONG                      returnLength = 0;
	NTSTATUS                   status = STATUS_UNSUCCESSFUL;

	if (!NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess(), &poolTagInformationBuffer, 0, &bufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
		Log(MSG_FATAL, "[MCPAC] [Fatal] Could not allocate memory.\n");
		NtTerminateProcess(NtCurrentProcess(), MEMORY_ALLOCATION_FAILED);
		__fastfail(MEMORY_ALLOCATION_FAILED);
	}

	status = NtQuerySystemInformation(SystemPoolTagInformation, poolTagInformationBuffer, static_cast<ULONG>(bufferSize), &returnLength);

	while (status == STATUS_INFO_LENGTH_MISMATCH) {
		SIZE_T freeSize = 0;
		if (!NT_SUCCESS(NtFreeVirtualMemory(NtCurrentProcess(), &poolTagInformationBuffer, &freeSize, MEM_RELEASE))) {
			NtTerminateProcess(NtCurrentProcess(), NT_API_INVOKE_FAILURE);
			__fastfail(NT_API_INVOKE_FAILURE);
		}

		poolTagInformationBuffer = 0;
		bufferSize += 0x10000;

		if (!NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess(), &poolTagInformationBuffer, 0, &bufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
			Log(MSG_FATAL, "[MCPAC] [Thread] [Fatal] Could not allocate memory.\n");
			NtTerminateProcess(NtCurrentProcess(), MEMORY_ALLOCATION_FAILED);
			__fastfail(MEMORY_ALLOCATION_FAILED);
		}

		status = NtQuerySystemInformation(SystemPoolTagInformation, poolTagInformationBuffer, static_cast<ULONG>(bufferSize), &returnLength);
	}

	if (!NT_SUCCESS(status)) {
		Log(MSG_FATAL, "[MCPAC] [Fatal] Cannot query the handle information, is there sufficient authority?\n");
		NtTerminateProcess(NtCurrentProcess(), NT_API_INVOKE_FAILURE);
		__fastfail(NT_API_INVOKE_FAILURE);
	}


	for (ULONG i = 0; i < reinterpret_cast<PSYSTEM_POOLTAG_INFORMATION>(poolTagInformationBuffer)->Count; i++) {
		if (!VerifyPool(&reinterpret_cast<PSYSTEM_POOLTAG_INFORMATION>(poolTagInformationBuffer)->TagInfo[i])) {
			NtTerminateProcess(NtCurrentProcess(), BAD_POOL_TAG);
			__fastfail(BAD_POOL_TAG);
		}
	}

	SIZE_T freeSize = 0;
	if (!NT_SUCCESS(NtFreeVirtualMemory(NtCurrentProcess(), &poolTagInformationBuffer, &freeSize, MEM_RELEASE))) {
		NtTerminateProcess(NtCurrentProcess(), NT_API_INVOKE_FAILURE);
		__fastfail(NT_API_INVOKE_FAILURE);
	}

	return TRUE;
}
