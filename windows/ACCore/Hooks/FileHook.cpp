#include "Hooks.h"

#include "../Verifier.h"


NTSTATUS NTAPI HookedNtCreateFile(_Out_ PHANDLE FileHandle, _In_ ACCESS_MASK DesiredAccess, _In_ POBJECT_ATTRIBUTES ObjectAttributes, _Out_ PIO_STATUS_BLOCK IoStatusBlock, _In_opt_ PLARGE_INTEGER AllocationSize, _In_ ULONG FileAttributes, _In_ ULONG ShareAccess, _In_ ULONG CreateDisposition, _In_ ULONG CreateOptions, _In_reads_bytes_opt_(EaLength) PVOID EaBuffer, _In_ ULONG EaLength) {
	if (VerifyFile(ObjectAttributes->ObjectName)) {
		return NtCreateFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
	} else {
		return STATUS_UNSUCCESSFUL;
	}
}