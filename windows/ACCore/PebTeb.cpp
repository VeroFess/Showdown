#include "../ThirdParty/phnt/phnt.h"

#include "Verifier.h"
#include "Errors.h"

VOID QueryModuleInInLoadOrderLinks() {
	auto peb = NtCurrentPeb();

	PLIST_ENTRY pHeadEntry = &peb->Ldr->InLoadOrderModuleList;
	PLIST_ENTRY pEntry = pHeadEntry->Flink;

	while (pEntry != pHeadEntry) {
		auto pLdrEntry = CONTAINING_RECORD(pEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

		if (!VerifyLibrary(&pLdrEntry->FullDllName)) {
			NtTerminateProcess(NtCurrentProcess(), UNABLE_TO_VERIFY_LOADED_LIBRARY);
			__fastfail(UNABLE_TO_VERIFY_LOADED_LIBRARY);
		}

		pEntry = pEntry->Flink;
	}
}