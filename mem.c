#include "asussaio.h"

BOOLEAN
AsusSAIOValidatePhysicalAddress(
    _In_ PHYSICAL_ADDRESS PhysicalAddress,
    _In_ SIZE_T Length
)
{
    // Prevent zero address, zero length, or excessively large mapping requests (> 16MB)
    if (PhysicalAddress.QuadPart == 0 || Length == 0 || Length > 0x1000000) {
        return FALSE;
    }

    // Check for 64-bit integer overflow
    if ((ULONG64)PhysicalAddress.QuadPart + Length < (ULONG64)PhysicalAddress.QuadPart) {
        return FALSE;
    }

    // Prohibit mapping low memory / IVT / BDA / kernel reserved physical memory (< 1MB)
    if (PhysicalAddress.QuadPart < 0x100000) {
        return FALSE;
    }

    return TRUE;
}

NTSTATUS
AsusSAIOReadPhysicalMemory(
    _In_ ULONG PhysicalAddress,
    _In_ UCHAR Length,
    _Out_ PULONG Data
)
{
    PHYSICAL_ADDRESS physAddr;
    PVOID mappedAddress;

    *Data = 0;

    if (Length != 1 && Length != 2 && Length != 4) {
        return STATUS_INVALID_PARAMETER;
    }

    physAddr.QuadPart = (LONGLONG)PhysicalAddress;

    // FIX: Validate physical address range to prevent arbitrary physical memory access
    if (!AsusSAIOValidatePhysicalAddress(physAddr, Length)) {
        return STATUS_ACCESS_DENIED;
    }

    mappedAddress = MmMapIoSpace(physAddr, Length, MmNonCached);
    if (mappedAddress == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    switch (Length) {
    case 1:
        *Data = *(PUCHAR)mappedAddress;
        break;
    case 2:
        *Data = *(PUSHORT)mappedAddress;
        break;
    case 4:
        *Data = *(PULONG)mappedAddress;
        break;
    default:
        break;
    }

    MmUnmapIoSpace(mappedAddress, Length);
    return STATUS_SUCCESS;
}
