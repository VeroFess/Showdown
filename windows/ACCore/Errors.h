#ifndef _ERRORS_H_
#define _ERRORS_H_
#include <windows.h>

constexpr UINT BINKLAC_ANTICHEAT_NOERROR       = 0x00000000;
//===================================================================================================================================================
// 0xBACE0001 - 0xBACE00FF: 未知错误,
// 不同的编码代表了不同的位置，这些错误代码只应该出现在catch中
constexpr UINT BINKLAC_ANTICHEAT_ERROR_UNKNOWN = 0xBACE0001;

//===================================================================================================================================================
// 0xB0E00100 - 0xB0E001FF: 程序内部错误，出现时应当是BUG

// 未知程序内部错误，用于特殊情况
constexpr UINT BINKLAC_ANTICHEAT_INTERNAL_ERROR = 0xBACE0100;
// 多线程情况下修改了其他线程的局部变量
// 这个变量同时出现在 .asm 文件中
constexpr UINT THREAD_CONFLICT                  = 0xBACE0101;
// NT API 的直接 SysCall 失败
constexpr UINT NT_API_INVOKE_FAILURE            = 0xBACE0102;
// 内存分配失败
constexpr UINT MEMORY_ALLOCATION_FAILED         = 0xBACE0103;

//===================================================================================================================================================
// 0xB0E00200 - 0xB0E002FF: 反外挂异常, 出现时为某一项反外挂检查未通过

// 未知反外挂异常，用于特殊情况
constexpr UINT UNKNOWN_ANTICHEAT_ERROR                    = 0xBACE0200;
// 线程的起点异常，通常发生在注入时
constexpr UINT UNEXPECTED_THREAD_START_ADDRESS            = 0xBACE0201;
// 线程起点页的属性被修改了，通常发生在注入和Hook时
constexpr UINT MODIFIED_THREAD_START_PAGE_ATTRIBUTE       = 0xBACE0202;
// 线程的起点页是不可执行的，不应该出现这种情况
constexpr UINT NON_EXECUTABLE_STARTING_PAGE               = 0xBACE0203;
// 线程的起点页是非IMAGE页，一定是远程注入
constexpr UINT STARTING_PAGE_ATTRIBUTE_IS_NOT_IMAGE       = 0xBACE0204;
// 线程的起点页是IMAGE，但是不知道它映射在了哪个DLL中
constexpr UINT UNABLE_TO_VERIFY_STARTING_PAGE_INFORMATION = 0xBACE0205;
// 加载到内存中的文件未经过验证
constexpr UINT UNABLE_TO_VERIFY_LOADED_FILE               = 0xBACE0206;
// 加载到内存中的动态库未经过验证
constexpr UINT UNABLE_TO_VERIFY_LOADED_LIBRARY            = 0xBACE0207;
// 在内核中存在非法的POOL，用于检查漏洞挂载
constexpr UINT BAD_POOL_TAG                               = 0xBACE0208;
#endif
