#ifndef _LOGGER_H_  // _LOGGER_HPP_ ? why .hpp? why not .h?
#define _LOGGER_H_

#include <iostream>
#include <locale>

#include "../ThirdParty/phnt/phnt.h"

// FIXME: use enum instead of INT if possible.
constexpr INT LOG_OFF = 0x0;
constexpr INT LOG_FATAL = 0x1;
constexpr INT LOG_ERROR = 0x2;
constexpr INT LOG_WARN = 0x3;
constexpr INT LOG_INFO = 0x4;
constexpr INT LOG_DEBUG = 0x5;
constexpr INT LOG_TRACE = 0x6;
constexpr INT LOG_TRACE_V = 0x7;
constexpr INT LOG_ALL = 0x8;
constexpr INT MSG_FATAL = 0x1;
constexpr INT MSG_ERROR = 0x2;
constexpr INT MSG_WARN = 0x3;
constexpr INT MSG_INFO = 0x4;
constexpr INT MSG_DEBUG = 0x5;
constexpr INT MSG_TRACE = 0x6;
constexpr INT MSG_TRACE_V = 0x7;
constexpr INT CURRENT_LOG_LEVEL = MSG_TRACE;
constexpr INT DEFAULT_STRING_LENGTH = 512;

static HANDLE LogFileHandle = INVALID_HANDLE_VALUE;

extern "C" {
static LPVOID LoggerMalloc(SIZE_T byteToAlloc) {
  LPVOID pBase = nullptr;
  SIZE_T allocSize = byteToAlloc, freeSize = 0;

  if (NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess(), &pBase, nullptr,
                                         &allocSize, MEM_COMMIT | MEM_RESERVE,
                                         PAGE_READWRITE))) {
    if (allocSize >= byteToAlloc) {
      return pBase;
    } else {
      NtFreeVirtualMemory(NtCurrentProcess(), &pBase, &freeSize, MEM_RELEASE);
      return nullptr;
    }
  }

  return nullptr;
}

static void LoggerFree(LPVOID pBase) {
  SIZE_T freeSize = 0;
  NtFreeVirtualMemory(NtCurrentProcess(), &pBase, &freeSize, MEM_RELEASE);
}

static void LoggerInitize(UNICODE_STRING file) {
  HANDLE hFile = nullptr;
  IO_STATUS_BLOCK FIOSB = {0};
  UNICODE_STRING USF = {0};
  OBJECT_ATTRIBUTES OAF = {0};

  auto pathBuffer = reinterpret_cast<PWCH>(LoggerMalloc(DEFAULT_STRING_LENGTH));

  ZeroMemory(pathBuffer, DEFAULT_STRING_LENGTH);
  wcscat_s(pathBuffer, DEFAULT_STRING_LENGTH / sizeof(WCHAR), L"\\??\\");
  wcsncat_s(pathBuffer, DEFAULT_STRING_LENGTH / sizeof(WCHAR), file.Buffer,
            file.Length);
  wcscat_s(pathBuffer, DEFAULT_STRING_LENGTH / sizeof(WCHAR), L"anticheat.log");

  USF.Buffer = pathBuffer;
  USF.MaximumLength = DEFAULT_STRING_LENGTH / sizeof(WCHAR);
  USF.Length = static_cast<USHORT>(
      wcsnlen_s(pathBuffer, DEFAULT_STRING_LENGTH / sizeof(WCHAR)) *
      sizeof(WCHAR));

  InitializeObjectAttributes(&OAF, &USF, OBJ_CASE_INSENSITIVE, nullptr,
                             nullptr);

  if (NT_SUCCESS(NtCreateFile(
          &hFile,
          STANDARD_RIGHTS_READ | FILE_READ_DATA | FILE_READ_ATTRIBUTES |
              FILE_READ_EA | STANDARD_RIGHTS_WRITE | FILE_WRITE_DATA |
              FILE_WRITE_ATTRIBUTES | FILE_WRITE_EA | FILE_APPEND_DATA |
              SYNCHRONIZE,
          &OAF, &FIOSB, nullptr, FILE_ATTRIBUTE_NORMAL,
          FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OVERWRITE_IF,
          FILE_SEQUENTIAL_ONLY | FILE_NON_DIRECTORY_FILE |
              FILE_SYNCHRONOUS_IO_NONALERT,
          nullptr, 0))) {
    LogFileHandle = hFile;
  }

  setlocale(LC_ALL, "");

  LoggerFree(pathBuffer);
}

static void Log(INT level, PCCH format, ...) {
  IO_STATUS_BLOCK FIOSB = {0};
  LARGE_INTEGER FO = {0};
  va_list arglist;

  if (CURRENT_LOG_LEVEL < level) {
    return;
  }

  auto stringBuffer =
      reinterpret_cast<PCH>(LoggerMalloc(DEFAULT_STRING_LENGTH));

  va_start(arglist, format);
  auto length =
      vsprintf_s(stringBuffer, DEFAULT_STRING_LENGTH, format, arglist);
  va_end(arglist);

  FO.HighPart = -1;
  FO.LowPart = FILE_WRITE_TO_END_OF_FILE;

  if (length != -1) {
    PPEB peb = NtCurrentPeb();
    auto StdOutput = peb->ProcessParameters->StandardOutput;
    if (StdOutput != INVALID_HANDLE_VALUE && StdOutput) {
      NtWriteFile(StdOutput, nullptr, nullptr, nullptr, &FIOSB,
                  reinterpret_cast<PVOID>(stringBuffer), length, &FO, nullptr);
    }

    NtWriteFile(LogFileHandle, nullptr, nullptr, nullptr, &FIOSB,
                reinterpret_cast<PVOID>(stringBuffer), length, &FO, nullptr);
  }

  LoggerFree(stringBuffer);
  return;
}
}
#endif