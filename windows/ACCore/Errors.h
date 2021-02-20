#pragma once

#define BINKLAC_ANTICHEAT_NOERROR                           0x00000000
//===================================================================================================================================================
//0xBACE0001 - 0xBACE00FF: 未知错误, 不同的编码代表了不同的位置，这些错误代码只应该出现在catch中
#define BINKLAC_ANTICHEAT_ERROR_UNKONW                      0xBACE0001

//===================================================================================================================================================
//0xB0E00100 - 0xB0E001FF: 程序内部错误，出现时应当是BUG

//未知程序内部错误，用于特殊情况
#define BINKLAC_ANTICHEAT_INTERNAL_ERROR                    0xBACE0100
//多线程情况下修改了其他线程的局部变量
//这个变量同时出现在 .asm 文件中
#define THREAD_CONFLICT                                     0xBACE0101
//NT API 的直接 SysCall 失败
#define NT_API_INVOKE_FAILURE                               0xBACE0102
//内存分配失败
#define MEMORY_ALLOCATION_FAILED                            0xBACE0103

//===================================================================================================================================================
//0xB0E00200 - 0xB0E002FF: 反外挂异常, 出现时为某一项反外挂检查未通过

//未知反外挂异常，用于特殊情况
#define UNKONW_ANTICHEAT_ERROR                              0xBACE0200
//线程的起点异常，通常发生在注入时
#define UNEXPECTED_THREAD_START_ADDRESS                     0xBACE0201
//线程起点页的属性被修改了，通常发生在注入和Hook时
#define MODIFIED_THREAD_START_PAGE_ATTRIBUTE                0xBACE0202
//线程的起点页是不可执行的，不应该出现这种情况
#define NON_EXECUTABLE_STARTING_PAGE                        0xBACE0203
//线程的起点页是非IMAGE页，一定是远程注入
#define STARTING_PAGE_ATTRIBUTE_IS_NOT_IMAGE                0xBACE0204
//线程的起点页是IMAGE，但是不知道它映射在了哪个DLL中
#define UNABLE_TO_VERIFY_STARTING_PAGE_INFORMATION          0xBACE0205
//加载到内存中的文件未经过验证
#define UNABLE_TO_VERIFY_LOADED_FILE                        0xBACE0206
//加载到内存中的动态库未经过验证
#define UNABLE_TO_VERIFY_LOADED_LIBRARY                     0xBACE0207