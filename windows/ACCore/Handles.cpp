#include "../ThirdParty/phnt/phnt.h"
#include "Logger.hpp"
#include "Errors.h"

#include <objbase.h>

#define PAGE_EXECUTE_FLAGS (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)

INT HandleQueryThread(PSYSTEM_HANDLE_TABLE_ENTRY_INFO HandleEntry) {
	SIZE_T                     bufferSize = 0x1000;
	HANDLE                     processHandle = INVALID_HANDLE_VALUE, duplicateHandle = INVALID_HANDLE_VALUE;
	PVOID                      objectTypeInformationBuffer = 0, objectNameInformationBuffer = 0, processNameInformationBuffer = 0, openedProcessNameInformationBuffer = 0;
	ULONG                      returnLength = 0;
	CLIENT_ID                  processId = { 0 };
	PROCESS_BASIC_INFORMATION  selfProcessInformation = { 0 }, targetProcessInformation = { 0 };
	OBJECT_ATTRIBUTES          processObject = { sizeof(OBJECT_ATTRIBUTES) };

	if (!NT_SUCCESS(NtQueryInformationProcess(NtCurrentProcess(), ProcessBasicInformation, &selfProcessInformation, sizeof(PROCESS_BASIC_INFORMATION), &returnLength))) {
		Log(MSG_FATAL, "[MCPAC] [Fatal] The operation of querying the basic information of the process failed.\n");
		NtTerminateProcess(NtCurrentProcess(), NT_API_INVOKE_FAILURE);
		__fastfail(NT_API_INVOKE_FAILURE);
	}

	if (!NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess(), &objectTypeInformationBuffer, 0, &bufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
		Log(MSG_FATAL, "[MCPAC] [Thread] [Fatal] Could not allocate memory.\n");
		NtTerminateProcess(NtCurrentProcess(), MEMORY_ALLOCATION_FAILED);
		__fastfail(MEMORY_ALLOCATION_FAILED);
	}

	if (!NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess(), &objectNameInformationBuffer, 0, &bufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
		Log(MSG_FATAL, "[MCPAC] [Thread] [Fatal] Could not allocate memory.\n");
		NtTerminateProcess(NtCurrentProcess(), MEMORY_ALLOCATION_FAILED);
		__fastfail(MEMORY_ALLOCATION_FAILED);
	}

	if (!NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess(), &processNameInformationBuffer, 0, &bufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
		Log(MSG_FATAL, "[MCPAC] [Thread] [Fatal] Could not allocate memory.\n");
		NtTerminateProcess(NtCurrentProcess(), MEMORY_ALLOCATION_FAILED);
		__fastfail(MEMORY_ALLOCATION_FAILED);
	}

	ZeroMemory(objectTypeInformationBuffer, bufferSize);
	ZeroMemory(objectNameInformationBuffer, bufferSize);

	processId.UniqueProcess = reinterpret_cast<HANDLE>(HandleEntry->UniqueProcessId);
	if (!NT_SUCCESS(NtOpenProcess(&processHandle, PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION, &processObject, &processId))) {
		goto CleanExit;
	}

	if (!NT_SUCCESS(NtQueryInformationProcess(processHandle, ProcessBasicInformation, &targetProcessInformation, sizeof(PROCESS_BASIC_INFORMATION), &returnLength))) {
		Log(MSG_FATAL, "[MCPAC] [Fatal] The operation of querying the basic information of the process failed.\n");
		NtTerminateProcess(NtCurrentProcess(), NT_API_INVOKE_FAILURE);
		__fastfail(NT_API_INVOKE_FAILURE);
	}

	if (!NT_SUCCESS(NtQueryInformationProcess(processHandle, ProcessImageFileName, processNameInformationBuffer, bufferSize, &returnLength))) {
		Log(MSG_FATAL, "[MCPAC] [Fatal] The operation of querying the basic information of the process failed.\n");
		NtTerminateProcess(NtCurrentProcess(), NT_API_INVOKE_FAILURE);
		__fastfail(NT_API_INVOKE_FAILURE);
	}

	if (!NT_SUCCESS(NtDuplicateObject(processHandle, reinterpret_cast<HANDLE>(HandleEntry->HandleValue), NtCurrentProcess(), &duplicateHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))) {
		Log(MSG_TRACE_V, "[MCPAC] [Error] The operation of duplicate object from %.*ls failed.\n", reinterpret_cast<PUNICODE_STRING>(processNameInformationBuffer)->Length / sizeof(WCHAR), reinterpret_cast<PUNICODE_STRING>(processNameInformationBuffer)->Buffer);
		goto CleanExit;
	}

	if (NT_SUCCESS(NtQueryObject(duplicateHandle, ObjectTypeInformation, objectTypeInformationBuffer, static_cast<ULONG>(bufferSize), NULL))) {
		Log(MSG_TRACE_V, "      Type: %.*ls\n", reinterpret_cast<POBJECT_TYPE_INFORMATION>(objectTypeInformationBuffer)->TypeName.Length / sizeof(WCHAR), reinterpret_cast<POBJECT_TYPE_INFORMATION>(objectTypeInformationBuffer)->TypeName.Buffer);
	} else {
		Log(MSG_ERROR, "[MCPAC] [Thread] [Error] Could not query object type information.\n");
		goto CleanExit;
	}

	if (targetProcessInformation.UniqueProcessId != selfProcessInformation.UniqueProcessId) {
		if (!_wcsnicmp(reinterpret_cast<POBJECT_TYPE_INFORMATION>(objectTypeInformationBuffer)->TypeName.Buffer, L"Process", reinterpret_cast<POBJECT_TYPE_INFORMATION>(objectTypeInformationBuffer)->TypeName.Length)) {
			PROCESS_BASIC_INFORMATION  openedProcessInformation = { 0 };

			if (!NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess(), &openedProcessNameInformationBuffer, 0, &bufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
				Log(MSG_FATAL, "[MCPAC] [Thread] [Fatal] Could not allocate memory.\n");
				NtTerminateProcess(NtCurrentProcess(), MEMORY_ALLOCATION_FAILED);
				__fastfail(MEMORY_ALLOCATION_FAILED);
			}

			if (!NT_SUCCESS(NtQueryInformationProcess(duplicateHandle, ProcessBasicInformation, &openedProcessInformation, sizeof(PROCESS_BASIC_INFORMATION), &returnLength))) {
				Log(MSG_ERROR, "[MCPAC] [Error] The operation of querying the basic information of the process failed.\n");
				goto CleanExit;
			}

			if (!NT_SUCCESS(NtQueryInformationProcess(duplicateHandle, ProcessImageFileName, openedProcessNameInformationBuffer, bufferSize, &returnLength))) {
				Log(MSG_ERROR, "[MCPAC] [Error] The operation of querying the image file name of the process failed.\n");
			}

			if (openedProcessInformation.UniqueProcessId == selfProcessInformation.UniqueProcessId) {
				if (NT_SUCCESS(NtDuplicateObject(processHandle, reinterpret_cast<HANDLE>(HandleEntry->HandleValue), NtCurrentProcess(), NULL, 0, FALSE, DUPLICATE_CLOSE_SOURCE))) {
					Log(MSG_TRACE, "[MCPAC] Closed self handle in process %.*ls!\n", "[MCPAC] target name: %.*ls\n", reinterpret_cast<PUNICODE_STRING>(processNameInformationBuffer)->Length / sizeof(WCHAR), reinterpret_cast<PUNICODE_STRING>(processNameInformationBuffer)->Buffer);
				}
			}
		}
	} else {
		if (!_wcsnicmp(reinterpret_cast<POBJECT_TYPE_INFORMATION>(objectTypeInformationBuffer)->TypeName.Buffer, L"File", reinterpret_cast<POBJECT_TYPE_INFORMATION>(objectTypeInformationBuffer)->TypeName.Length)) {
			IO_STATUS_BLOCK            FIOSB = { 0 };
			FILE_STANDARD_INFORMATION  FI = { 0 };

			if (NT_SUCCESS(NtQueryInformationFile(reinterpret_cast<HANDLE>(HandleEntry->HandleValue), &FIOSB, &FI, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation)) && !FI.Directory) {
				Log(MSG_INFO, "      An open file was found: %.*ls\n", reinterpret_cast<POBJECT_NAME_INFORMATION>(objectNameInformationBuffer)->Name.Length, reinterpret_cast<POBJECT_NAME_INFORMATION>(objectNameInformationBuffer)->Name.Buffer);

				//TODO: Verify the opened file.
			}
		}

		if (!_wcsnicmp(reinterpret_cast<POBJECT_TYPE_INFORMATION>(objectTypeInformationBuffer)->TypeName.Buffer, L"Thread", reinterpret_cast<POBJECT_TYPE_INFORMATION>(objectTypeInformationBuffer)->TypeName.Length)) {
			LPVOID                     StartAddress = nullptr;
			ULONG                      returnLength = 0;

			if (NT_SUCCESS(NtQueryInformationThread(reinterpret_cast<HANDLE>(HandleEntry->HandleValue), ThreadQuerySetWin32StartAddress, &StartAddress, sizeof(LPVOID), &returnLength))) {

				if (StartAddress == HandleQueryThread) {
					goto CleanExit;
				} else {
					Log(MSG_DEBUG, "[MCPAC] [Thread] Start valid thread: 0x%016X\n", HandleEntry->HandleValue);
				}

				if (StartAddress == LoadLibraryA || StartAddress == LoadLibraryW || StartAddress == LoadLibraryExA || StartAddress == LoadLibraryExW || StartAddress == CoLoadLibrary) {
					Log(MSG_FATAL, "[MCPAC] [Thread] [Fatal] Error valid thread: 0x%016X\n", StartAddress);
					NtTerminateProcess(NtCurrentProcess(), UNEXPECTED_THREAD_START_ADDRESS);
					__fastfail(UNEXPECTED_THREAD_START_ADDRESS);
				}

				MEMORY_BASIC_INFORMATION memoryBasicInformation = { 0 };
				SIZE_T returnLengthVM = 0;

				NTSTATUS status = 0;

				if (NT_SUCCESS(NtQueryVirtualMemory(NtCurrentProcess(), StartAddress, MemoryBasicInformation, &memoryBasicInformation, sizeof(MEMORY_BASIC_INFORMATION), &returnLengthVM))) {
					if (!(memoryBasicInformation.AllocationProtect != memoryBasicInformation.Protect)) {
						Log(MSG_FATAL, "[MCPAC] [Thread] [Fatal] Error valid thread: 0x%016X\n", HandleEntry->HandleValue);
						NtTerminateProcess(NtCurrentProcess(), MODIFIED_THREAD_START_PAGE_ATTRIBUTE);
						__fastfail(MODIFIED_THREAD_START_PAGE_ATTRIBUTE);
					}

					if (!(memoryBasicInformation.AllocationProtect & PAGE_EXECUTE_FLAGS)) {
						Log(MSG_FATAL, "[MCPAC] [Thread] [Fatal] Error valid thread: 0x%016X\n", HandleEntry->HandleValue);
						NtTerminateProcess(NtCurrentProcess(), NON_EXECUTABLE_STARTING_PAGE);
						__fastfail(NON_EXECUTABLE_STARTING_PAGE);
					}

					if (memoryBasicInformation.Type != SEC_IMAGE) {
						Log(MSG_FATAL, "[MCPAC] [Thread] [Fatal] Error valid thread: 0x%016X\n", HandleEntry->HandleValue);
						NtTerminateProcess(NtCurrentProcess(), STARTING_PAGE_ATTRIBUTE_IS_NOT_IMAGE);
						__fastfail(STARTING_PAGE_ATTRIBUTE_IS_NOT_IMAGE);
					}

					auto memoryMappedFilenameInformationBuffer = reinterpret_cast<PUNICODE_STRING>(malloc(bufferSize));
					if (memoryMappedFilenameInformationBuffer == nullptr) {
						NtTerminateProcess(NtCurrentProcess(), MEMORY_ALLOCATION_FAILED);
						__fastfail(MEMORY_ALLOCATION_FAILED);
					}

					if (NT_SUCCESS(status = NtQueryVirtualMemory(NtCurrentProcess(), memoryBasicInformation.AllocationBase, MemoryMappedFilenameInformation, memoryMappedFilenameInformationBuffer, bufferSize, &returnLengthVM))) {
						Log(MSG_DEBUG, "[MCPAC] [Thread] The start address of thread <0x%016X> is in module %.*ls.\n", HandleEntry->HandleValue, memoryMappedFilenameInformationBuffer->Length, memoryMappedFilenameInformationBuffer->Buffer);
					} else {
						Log(MSG_FATAL, "[MCPAC] [Thread] [Fatal] The memory attribute of this area is an executable program, but related information cannot be queried.");
						NtTerminateProcess(NtCurrentProcess(), UNABLE_TO_VERIFY_STARTING_PAGE_INFORMATION);
						__fastfail(UNABLE_TO_VERIFY_STARTING_PAGE_INFORMATION);
					}

					free(memoryMappedFilenameInformationBuffer);
				}

			}
		}
	}

CleanExit:
	if (objectTypeInformationBuffer != nullptr) {
		SIZE_T freeSize = 0;
		NtFreeVirtualMemory(NtCurrentProcess(), &objectTypeInformationBuffer, &freeSize, MEM_RELEASE);
	}

	if (objectNameInformationBuffer != nullptr) {
		SIZE_T freeSize = 0;
		NtFreeVirtualMemory(NtCurrentProcess(), &objectNameInformationBuffer, &freeSize, MEM_RELEASE);
	}

	if (processNameInformationBuffer != nullptr) {
		SIZE_T freeSize = 0;
		NtFreeVirtualMemory(NtCurrentProcess(), &processNameInformationBuffer, &freeSize, MEM_RELEASE);
	}

	if (openedProcessNameInformationBuffer != nullptr) {
		SIZE_T freeSize = 0;
		NtFreeVirtualMemory(NtCurrentProcess(), &openedProcessNameInformationBuffer, &freeSize, MEM_RELEASE);
	}

	if (processHandle != INVALID_HANDLE_VALUE) {
		NtClose(processHandle);
	}

	if (duplicateHandle != INVALID_HANDLE_VALUE) {
		NtClose(duplicateHandle);
	}

	//TODO: Make Sure the check thread alive

	return 0;
}

BOOLEAN VerifyProcessHandles() {
	BOOL                       returnValue = TRUE;
	PVOID                      systemHandleInformationBuffer = nullptr;
	SIZE_T                     pageSize = 0x10000, bufferSize = 0x1000;
	ULONG                      returnLength = 0;
	NTSTATUS                   status = STATUS_UNSUCCESSFUL;
	PROCESS_BASIC_INFORMATION  processInformation = { 0 };


	PSYSTEM_HANDLE_INFORMATION systemHandleInformation = 0;

	Log(MSG_DEBUG, "[MCPAC] Retrieving hanles opened by the process ...\n");

	if (!NT_SUCCESS(NtQueryInformationProcess(NtCurrentProcess(), ProcessBasicInformation, &processInformation, sizeof(PROCESS_BASIC_INFORMATION), &returnLength))) {
		Log(MSG_FATAL, "[MCPAC] [Fatal] The operation of querying the basic information of the process failed.\n");
		NtTerminateProcess(NtCurrentProcess(), NT_API_INVOKE_FAILURE);
		__fastfail(NT_API_INVOKE_FAILURE);
	}

	if (!NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess(), &systemHandleInformationBuffer, 0, &pageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
		Log(MSG_FATAL, "[MCPAC] [Fatal] Could not allocate memory.\n");
		NtTerminateProcess(NtCurrentProcess(), MEMORY_ALLOCATION_FAILED);
		__fastfail(MEMORY_ALLOCATION_FAILED);
	}

	status = NtQuerySystemInformation(SystemHandleInformation, systemHandleInformationBuffer, static_cast<ULONG>(pageSize), &returnLength);

	while (status == STATUS_INFO_LENGTH_MISMATCH) {
		SIZE_T freeSize = 0;
		if (!NT_SUCCESS(NtFreeVirtualMemory(NtCurrentProcess(), &systemHandleInformationBuffer, &freeSize, MEM_RELEASE))) {
			NtTerminateProcess(NtCurrentProcess(), NT_API_INVOKE_FAILURE);
			__fastfail(NT_API_INVOKE_FAILURE);
		}
		systemHandleInformationBuffer = 0;
		pageSize += 0x10000;

		if (!NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess(), &systemHandleInformationBuffer, 0, &pageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
			Log(MSG_FATAL, "[MCPAC] [Thread] [Fatal] Could not allocate memory.\n");
			NtTerminateProcess(NtCurrentProcess(), MEMORY_ALLOCATION_FAILED);
			__fastfail(MEMORY_ALLOCATION_FAILED);
		}

		status = NtQuerySystemInformation(SystemHandleInformation, systemHandleInformationBuffer, static_cast<ULONG>(pageSize), &returnLength);
	}

	if (!NT_SUCCESS(status)) {
		Log(MSG_FATAL, "[MCPAC] [Fatal] Cannot query the handle information, is there sufficient authority?\n");
		NtTerminateProcess(NtCurrentProcess(), NT_API_INVOKE_FAILURE);
		__fastfail(NT_API_INVOKE_FAILURE);
	}

	systemHandleInformation = reinterpret_cast<PSYSTEM_HANDLE_INFORMATION>(systemHandleInformationBuffer);

	for (ULONG i = 0; i < systemHandleInformation->NumberOfHandles; i++) {
		HANDLE handle;
		if (NT_SUCCESS(NtCreateThreadEx(&handle, THREAD_ALL_ACCESS, NULL, NtCurrentProcess(), HandleQueryThread, &systemHandleInformation->Handles[i], FALSE, NULL, NULL, NULL, NULL))) {
			LARGE_INTEGER waitTime;
			waitTime.QuadPart = -60000000;
			auto status = NtWaitForSingleObject(handle, FALSE, &waitTime);
			if (!NT_SUCCESS(status) || status == STATUS_TIMEOUT) {
				Log(MSG_WARN, "[MCPAC] [Warning] Thread timed out, skip this item.\n");
				NtTerminateThread(handle, -1);
			}

			NtClose(handle);
		} else {
			Log(MSG_FATAL, "[MCPAC] [Fatal] Cannot create a new thread, there are no resources?\n");
			NtTerminateProcess(NtCurrentProcess(), NT_API_INVOKE_FAILURE);
			__fastfail(NT_API_INVOKE_FAILURE);
		}
	}

	if (systemHandleInformationBuffer != nullptr) {
		SIZE_T freeSize = 0;
		NtFreeVirtualMemory(NtCurrentProcess(), &systemHandleInformationBuffer, &freeSize, MEM_RELEASE);

	}

	return returnValue;
}