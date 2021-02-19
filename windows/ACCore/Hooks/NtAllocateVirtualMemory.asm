.data
NtAllocateVirtualMemoryAddress dq 0

EXTERN NtAllocateVirtualMemoryContextReturnAddress:QWORD
EXTERN NtAllocateVirtualMemoryContextFunctionRIP:QWORD
EXTERN NtAllocateVirtualMemoryContextRbp:QWORD
EXTERN NtAllocateVirtualMemoryContextRsp:QWORD
EXTERN NtAllocateVirtualMemoryLock:QWORD
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
_SetContextNtAllocateVirtualMemoryAddress proc
	mov             NtAllocateVirtualMemoryAddress, rcx
	ret
_SetContextNtAllocateVirtualMemoryAddress endp

_SaveContextNtAllocateVirtualMemory proc
	push            rbx
AcquireContextLock:
	push            rcx
	mov             rax, 0
	mov             rbx, 1
    lock cmpxchg    QWORD ptr [TLSACC(NtAllocateVirtualMemoryLock, rcx, CleanAndContinue1)], rbx
	pop             rcx
	jne             BOOM
	pop             rbx
    pop             rax
	push            rcx
    mov             [TLSACC(NtAllocateVirtualMemoryContextReturnAddress, rcx, CleanAndContinue2)], rax
	pop             rcx
    push            rax
    call            QueryCurrentRIP
QueryCurrentRIP:
	pop             rax
	push            rcx
	mov             [TLSACC(NtAllocateVirtualMemoryContextFunctionRIP, rcx, CleanAndContinue2)], rax
	pop             rcx
	mov             rax, rsp
	push            rcx
	mov             [TLSACC(NtAllocateVirtualMemoryContextRbp, rcx, CleanAndContinue2)], rbp
	mov             [TLSACC(NtAllocateVirtualMemoryContextRsp, rcx, CleanAndContinue2)], rax
	pop             rcx
	mov             rax, NtAllocateVirtualMemoryAddress
	jmp             rax
CleanAndContinue1:
	pop             rcx
	pop             rbx
	mov             rax, NtAllocateVirtualMemoryAddress
	jmp             rax
CleanAndContinue2:
	pop             rcx
	mov             rax, NtAllocateVirtualMemoryAddress
	jmp             rax
BOOM:
	mov             rcx, -1
	mov             edx, 0BACE0101h
	call            NtTerminateProcess
_SaveContextNtAllocateVirtualMemory endp

_QueryContextNtAllocateVirtualMemoryCaller proc
	GETTLSVAR       NtAllocateVirtualMemoryContextReturnAddress, rax, ContinueFunctionCall
	jmp short       ContinueFunctionCall
ContinueFunctionCall:
	ret
_QueryContextNtAllocateVirtualMemoryCaller endp

_QueryContextNtAllocateVirtualMemoryRip proc
	GETTLSVAR       NtAllocateVirtualMemoryContextFunctionRIP, rax, ContinueFunctionCall
	jmp short       ContinueFunctionCall
ContinueFunctionCall:
	ret
_QueryContextNtAllocateVirtualMemoryRip endp

_QueryContextNtAllocateVirtualMemoryRbp proc
	GETTLSVAR       NtAllocateVirtualMemoryContextRbp, rax, ContinueFunctionCall
	jmp short       ContinueFunctionCall
ContinueFunctionCall:
	ret
_QueryContextNtAllocateVirtualMemoryRbp endp

_QueryContextNtAllocateVirtualMemoryRsp proc
	GETTLSVAR       NtAllocateVirtualMemoryContextRsp, rax, ContinueFunctionCall
	jmp short       ContinueFunctionCall
ContinueFunctionCall:
	ret
_QueryContextNtAllocateVirtualMemoryRsp endp

_ReleaseNtAllocateVirtualMemoryContextLock proc
    mov             rax, 0
	push            rcx
    lock xchg       QWORD ptr [TLSACC(NtAllocateVirtualMemoryLock, rcx, ContinueFunctionCall)], rax
	jmp short       ContinueFunctionCall
ContinueFunctionCall:
	pop             rcx
	ret
_ReleaseNtAllocateVirtualMemoryContextLock endp

end