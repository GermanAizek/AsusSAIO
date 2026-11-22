#include "asussaio.h"

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
