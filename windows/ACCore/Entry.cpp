#include "../ThirdParty/phnt/phnt.h"
#include "Logger.hpp"
#include "Hooks/Hooks.h"

PVOID WINAPI HookedAddVectoredExceptionHandler(_In_ ULONG First, _In_ PVECTORED_EXCEPTION_HANDLER Handler) {
	return nullptr;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
	auto peb = NtCurrentPeb();
	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			LoggerInitize(peb->ProcessParameters->CurrentDirectory.DosPath);

			AddVectoredExceptionHandler(TRUE, VectoredHandler);
			NtApiBreakAndHook(AddVectoredExceptionHandler, HookedAddVectoredExceptionHandler);

			_SetContextNtProtectVirtualMemoryAddress(HookedNtProtectVirtualMemory);
			_SetContextNtAllocateVirtualMemoryAddress(HookedNtAllocateVirtualMemory);
			NtApiBreakAndHook(NtCreateSection, HookedNtCreateSection);
			NtApiHook(NtProtectVirtualMemory, _SaveContextNtProtectVirtualMemory);
			NtApiHook(NtAllocateVirtualMemory, _SaveContextNtAllocateVirtualMemory);
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}