#pragma once
#include "../../ThirdParty/phnt/phnt.h"

BOOLEAN NtApiHook(PVOID functionToHook, PVOID targetFunction) {
	ULONG oldProtect = 0;
	SIZE_T sizeToModify = 11;

	auto functionStartAddress = functionToHook;
	if (NT_SUCCESS(NtProtectVirtualMemory(NtCurrentProcess(), &functionStartAddress, &sizeToModify, PAGE_READWRITE, &oldProtect))) {
		reinterpret_cast<PBYTE>(functionToHook)[0] = 0x48;
		reinterpret_cast<PBYTE>(functionToHook)[1] = 0xB8;
		memcpy(reinterpret_cast<PBYTE>(functionToHook) + 2, &targetFunction, 8);
		reinterpret_cast<PBYTE>(functionToHook)[10] = 0xFF;
		reinterpret_cast<PBYTE>(functionToHook)[11] = 0xE0;
		functionStartAddress = functionToHook;
		return NT_SUCCESS(NtProtectVirtualMemory(NtCurrentProcess(), &functionStartAddress, &sizeToModify, PAGE_EXECUTE_READ, &oldProtect));
	} else {
		return FALSE;
	}
}

BOOLEAN NtApiBreakAndHook(PVOID functionToHook, PVOID targetFunction) {
	ULONG oldProtect = 0;
	SIZE_T sizeToModify = 12;

	auto functionStartAddress = functionToHook;
	if (NT_SUCCESS(NtProtectVirtualMemory(NtCurrentProcess(), &functionStartAddress, &sizeToModify, PAGE_READWRITE, &oldProtect))) {
		reinterpret_cast<PBYTE>(functionToHook)[0] = 0xCC;
		reinterpret_cast<PBYTE>(functionToHook)[1] = 0x48;
		reinterpret_cast<PBYTE>(functionToHook)[2] = 0xB8;
		memcpy(reinterpret_cast<PBYTE>(functionToHook) + 3, &targetFunction, 8);
		reinterpret_cast<PBYTE>(functionToHook)[11] = 0xFF;
		reinterpret_cast<PBYTE>(functionToHook)[12] = 0xE0;
		functionStartAddress = functionToHook;
		return NT_SUCCESS(NtProtectVirtualMemory(NtCurrentProcess(), &functionStartAddress, &sizeToModify, PAGE_EXECUTE_READ, &oldProtect));
	} else {
		return FALSE;
	}
}