#pragma once

#include "../ThirdParty/phnt/phnt.h"
#include <iostream>
#include <locale>

#define LOG_OFF     0x0
#define LOG_FATAL   0x1
#define LOG_ERROR   0x2
#define LOG_WARN    0x3
#define LOG_INFO    0x4
#define LOG_DEBUG   0x5
#define LOG_TRACE   0x6 
#define LOG_TRACE_V 0x7
#define LOG_ALL     0x8
#define MSG_FATAL   0x1
#define MSG_ERROR   0x2
#define MSG_WARN    0x3
#define MSG_INFO    0x4
#define MSG_DEBUG   0x5
#define MSG_TRACE   0x6 
#define MSG_TRACE_V 0x7
#define CURRENT_LOG_LEVEL MSG_TRACE
#define DEFAULT_STRING_LENGTH 512

static HANDLE LogFileHandle = INVALID_HANDLE_VALUE;

extern "C" static LPVOID LoggerMalloc(SIZE_T byteToAlloc) {
	LPVOID pBase = nullptr;
	SIZE_T allocSize = byteToAlloc, freeSize = 0;

	if (NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess(), &pBase, NULL, &allocSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE))) {
		if (allocSize >= byteToAlloc) {
			return pBase;
		} else {
			NtFreeVirtualMemory(NtCurrentProcess(), &pBase, &freeSize, MEM_RELEASE);
			return nullptr;
		}
	}

	return nullptr;
}

static VOID LoggerFree(LPVOID pBase) {
	SIZE_T freeSize = 0;
	NtFreeVirtualMemory(NtCurrentProcess(), &pBase, &freeSize, MEM_RELEASE);
}

extern "C" static  VOID LoggerInitize(UNICODE_STRING file) {
	HANDLE            hFile = NULL;
	IO_STATUS_BLOCK   FIOSB = { 0 };
	UNICODE_STRING    USF = { 0 };
	OBJECT_ATTRIBUTES OAF = { 0 };

	auto pathBuffer = reinterpret_cast<PWCH>(LoggerMalloc(DEFAULT_STRING_LENGTH));

	ZeroMemory(pathBuffer, DEFAULT_STRING_LENGTH);
	wcscat_s(pathBuffer, DEFAULT_STRING_LENGTH / sizeof(WCHAR), L"\\??\\");
	wcsncat_s(pathBuffer, DEFAULT_STRING_LENGTH / sizeof(WCHAR), file.Buffer, file.Length);
	wcscat_s(pathBuffer, DEFAULT_STRING_LENGTH / sizeof(WCHAR), L"anticheat.log");

	USF.Buffer = pathBuffer;
	USF.MaximumLength = DEFAULT_STRING_LENGTH / sizeof(WCHAR);
	USF.Length = static_cast<USHORT>(wcsnlen_s(pathBuffer, DEFAULT_STRING_LENGTH / sizeof(WCHAR)) * sizeof(WCHAR));

	InitializeObjectAttributes(&OAF, &USF, OBJ_CASE_INSENSITIVE, NULL, NULL);

	if (NT_SUCCESS(NtCreateFile(&hFile, STANDARD_RIGHTS_READ | FILE_READ_DATA | FILE_READ_ATTRIBUTES | FILE_READ_EA | STANDARD_RIGHTS_WRITE | FILE_WRITE_DATA | FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | FILE_APPEND_DATA | SYNCHRONIZE, &OAF, &FIOSB, NULL, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OVERWRITE_IF, FILE_SEQUENTIAL_ONLY | FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0))) {
		LogFileHandle = hFile;
	}

	setlocale(LC_ALL, "");

	LoggerFree(pathBuffer);
}

extern "C" static VOID Log(INT level, PCCH format, ...) {
	IO_STATUS_BLOCK FIOSB = { 0 };
	LARGE_INTEGER   FO = { 0 };
	va_list         arglist;


	if (CURRENT_LOG_LEVEL < level) {
		return;
	}

	auto stringBuffer = reinterpret_cast<PCH>(LoggerMalloc(DEFAULT_STRING_LENGTH));

	va_start(arglist, format);
	auto length = vsprintf_s(stringBuffer, DEFAULT_STRING_LENGTH, format, arglist);
	va_end(arglist);

	FO.HighPart = -1;
	FO.LowPart = FILE_WRITE_TO_END_OF_FILE;

	if (length != -1) {
		PPEB peb = NtCurrentPeb();
		auto StdOutput = peb->ProcessParameters->StandardOutput;
		if (StdOutput != INVALID_HANDLE_VALUE && StdOutput != NULL) {
			NtWriteFile(StdOutput, NULL, NULL, NULL, &FIOSB, reinterpret_cast<PVOID>(stringBuffer), length, &FO, NULL);
		}

		NtWriteFile(LogFileHandle, NULL, NULL, NULL, &FIOSB, reinterpret_cast<PVOID>(stringBuffer), length, &FO, NULL);
	}

	LoggerFree(stringBuffer);
	return;
}