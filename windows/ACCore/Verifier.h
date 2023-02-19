#ifndef _VERIFIER_H_
#define _VERIFIER_H_
#include "../ThirdParty/phnt/phnt.h"

BOOLEAN VerifyFile(PUNICODE_STRING file);
BOOLEAN VerifyLibrary(PUNICODE_STRING file);
BOOLEAN VerifyPool(PSYSTEM_POOLTAG poolTag);
#endif
