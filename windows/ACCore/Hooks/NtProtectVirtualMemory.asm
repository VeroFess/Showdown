.data
NtProtectVirtualMemoryAddress dq 0

EXTERN NtProtectVirtualMemoryContextReturnAddress:QWORD
EXTERN NtProtectVirtualMemoryContextFunctionRIP:QWORD
EXTERN NtProtectVirtualMemoryContextRbp:QWORD
EXTERN NtProtectVirtualMemoryContextRsp:QWORD
EXTERN NtProtectVirtualMemoryLock:QWORD
EXTERN NtTerminateProcess:PROC
EXTERN _tls_index:DWORD

TLSACC macro variable:REQ, register:REQ, uninitializedLabel:REQ
	LOCAL           Clean, Exit
	sub             rsp, 8
	push            r12
	push            rcx
	mov             r12, gs:[88]
	cmp             r12, 0
	je short        Clean
    mov             ecx, _tls_index
    mov             r12, [r12 + rcx * 8]
	mov             ecx, SECTIONREL variable
    add             r12, rcx
	mov             QWORD PTR [rsp + 10h], r12
	pop             rcx
	pop             r12
	mov             register, QWORD PTR [rsp]
	add             rsp, 8
	jmp short       Exit
Clean:
	pop             rcx
	pop             r12
	add             rsp, 8
	jmp             uninitializedLabel
Exit:
	exitm           <register>
endm

SETTLSVAR macro variable:REQ, register:REQ, uninitializedLabel:REQ
	push            r11
	mov             [TLSACC(variable, r11, uninitializedLabel)], register
	pop             r11
endm

GETTLSVAR macro variable:REQ, register:REQ, uninitializedLabel:REQ
	push            r11
	mov             register, [TLSACC(variable, r11, uninitializedLabel)]
	pop             r11
endm

.code
_SetContextNtProtectVirtualMemoryAddress proc
	mov             NtProtectVirtualMemoryAddress, rcx
	ret
_SetContextNtProtectVirtualMemoryAddress endp

_SaveContextNtProtectVirtualMemory proc
	push            rbx
AcquireContextLock:
	push            rcx
	mov             rax, 0
	mov             rbx, 1
    lock cmpxchg    QWORD ptr [TLSACC(NtProtectVirtualMemoryLock, rcx, CleanAndContinue1)], rbx
	pop             rcx
	jne             BOOM
	pop             rbx
    pop             rax
	push            rcx
    mov             [TLSACC(NtProtectVirtualMemoryContextReturnAddress, rcx, CleanAndContinue2)], rax
	pop             rcx
    push            rax
    call            QueryCurrentRIP
QueryCurrentRIP:
	pop             rax
	push            rcx
	mov             [TLSACC(NtProtectVirtualMemoryContextFunctionRIP, rcx, CleanAndContinue2)], rax
	pop             rcx
	mov             rax, rsp
	push            rcx
	mov             [TLSACC(NtProtectVirtualMemoryContextRbp, rcx, CleanAndContinue2)], rbp
	mov             [TLSACC(NtProtectVirtualMemoryContextRsp, rcx, CleanAndContinue2)], rax
	pop             rcx
	mov             rax, NtProtectVirtualMemoryAddress
	jmp             rax
CleanAndContinue1:
	pop             rcx
	pop             rbx
	mov             rax, NtProtectVirtualMemoryAddress
	jmp             rax
CleanAndContinue2:
	pop             rcx
	mov             rax, NtProtectVirtualMemoryAddress
	jmp             rax
BOOM:
	mov             rcx, -1
	mov             edx, 0BACE0101h
	call            NtTerminateProcess
_SaveContextNtProtectVirtualMemory endp

_QueryContextNtProtectVirtualMemoryCaller proc
	GETTLSVAR       NtProtectVirtualMemoryContextReturnAddress, rax, ContinueFunctionCall
	jmp short       ContinueFunctionCall
ContinueFunctionCall:
	ret
_QueryContextNtProtectVirtualMemoryCaller endp

_QueryContextNtProtectVirtualMemoryRip proc
	GETTLSVAR       NtProtectVirtualMemoryContextFunctionRIP, rax, ContinueFunctionCall
	jmp short       ContinueFunctionCall
ContinueFunctionCall:
	ret
_QueryContextNtProtectVirtualMemoryRip endp

_QueryContextNtProtectVirtualMemoryRbp proc
	GETTLSVAR       NtProtectVirtualMemoryContextRbp, rax, ContinueFunctionCall
	jmp short       ContinueFunctionCall
ContinueFunctionCall:
	ret
_QueryContextNtProtectVirtualMemoryRbp endp

_QueryContextNtProtectVirtualMemoryRsp proc
	GETTLSVAR       NtProtectVirtualMemoryContextRsp, rax, ContinueFunctionCall
	jmp short       ContinueFunctionCall
ContinueFunctionCall:
	ret
_QueryContextNtProtectVirtualMemoryRsp endp

_ReleaseNtProtectVirtualMemoryContextLock proc
    mov             rax, 0
	push            rcx
    lock xchg       QWORD ptr [TLSACC(NtProtectVirtualMemoryLock, rcx, ContinueFunctionCall)], rax
	jmp short       ContinueFunctionCall
ContinueFunctionCall:
	pop             rcx
	ret
_ReleaseNtProtectVirtualMemoryContextLock endp

end