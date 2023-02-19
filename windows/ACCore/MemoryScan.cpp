#include "../ThirdParty/phnt/phnt.h"
#include "Errors.h"
#include "Logger.hpp"
#include "Verifier.h"

BOOL QueryVirtualMemoryForHiddenLibrary() {
    SIZE_T                   returnedLength = 0, bufferSize = 0x1000;
    PVOID                    memoryMappedFilenameInformationBuffer = 0;
    MEMORY_BASIC_INFORMATION memoryBasicInformation                = { 0 };

    if (!NT_SUCCESS(NtAllocateVirtualMemory(
            NtCurrentProcess(), &memoryMappedFilenameInformationBuffer, 0,
            &bufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
        Log(MSG_FATAL, "[MCPAC] [Fatal] Could not allocate memory.\n");
        NtTerminateProcess(NtCurrentProcess(), MEMORY_ALLOCATION_FAILED);
        __fastfail(MEMORY_ALLOCATION_FAILED);
    }

    for (ULONG64 StepAddress = 0; StepAddress < 0x00007FFFFFFFFFFF;
         StepAddress++) {
        if (NT_SUCCESS(NtQueryVirtualMemory(
                NtCurrentProcess(), (PVOID)StepAddress, MemoryBasicInformation,
                &memoryBasicInformation, sizeof(MEMORY_BASIC_INFORMATION),
                &returnedLength))) {
            if (memoryBasicInformation.Type == SEC_IMAGE) {
                ZeroMemory(memoryMappedFilenameInformationBuffer, bufferSize);
                if (NT_SUCCESS(
                        NtQueryVirtualMemory(NtCurrentProcess(), (PVOID)StepAddress, MemoryMappedFilenameInformation, memoryMappedFilenameInformationBuffer, bufferSize, &returnedLength))) {
                    if (!VerifyLibrary(reinterpret_cast<PUNICODE_STRING>(
                            memoryMappedFilenameInformationBuffer))) {
                        NtTerminateProcess(NtCurrentProcess(), UNABLE_TO_VERIFY_LOADED_LIBRARY);
                        __fastfail(UNABLE_TO_VERIFY_LOADED_LIBRARY);
                    }
                } else {
                    Log(MSG_FATAL,
                        "[MCPAC] [Fatal] The memory attribute of this area is an "
                        "executable program, but related information cannot be queried.");
                    NtTerminateProcess(NtCurrentProcess(), UNABLE_TO_VERIFY_STARTING_PAGE_INFORMATION);
                    __fastfail(UNABLE_TO_VERIFY_STARTING_PAGE_INFORMATION);
                }
            }

            StepAddress += memoryBasicInformation.RegionSize;
        } else {
            StepAddress += 0x1000;
        }
    }

    if (memoryMappedFilenameInformationBuffer) {
        bufferSize = 0;
        NtFreeVirtualMemory(NtCurrentProcess(), &memoryMappedFilenameInformationBuffer, &bufferSize, MEM_FREE);
    }

    return true;
}
