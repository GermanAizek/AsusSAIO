#include "asussaio.h"

BOOLEAN
AsusSAIOValidatePciAccess(
    _In_ UCHAR Bus,
    _In_ UCHAR Device,
    _In_ UCHAR Function,
    _In_ UCHAR Offset
)
{
    UNREFERENCED_PARAMETER(Bus);

    if (Device > 31 || Function > 7) {
        return FALSE;
    }

    // Offset must be 4-byte aligned and within standard PCI configuration space
    if ((Offset & 3) != 0 || Offset > 0xFC) {
        return FALSE;
    }

    return TRUE;
}

NTSTATUS
AsusSAIOReadPciConfig(
    _In_ UCHAR Bus,
    _In_ UCHAR Device,
    _In_ UCHAR Function,
    _In_ UCHAR Offset,
    _Out_ PULONG Data
)
{
    ULONG slot = (Function << 5) | (Device & 0x1F);
    ULONG bytesRead;

    *Data = 0;

    // FIX: Validate PCI parameters bounds
    if (!AsusSAIOValidatePciAccess(Bus, Device, Function, Offset)) {
        return STATUS_ACCESS_DENIED;
    }

    bytesRead = HalGetBusDataByOffset(
        PCIConfiguration,
        Bus,
        slot,
        Data,
        Offset,
        sizeof(ULONG)
    );

    if (bytesRead == 0) {
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}
